const PNG = require('pngjs').PNG;
const fs = require("fs");

const assetPath = "assets/raw/";
const compiledPath = "assets/compiled/";
const files = fs.readdirSync(assetPath, { withFileTypes: true });

function RGBtoCIELAB(r, g, b) {
  //Based on the method suggested here:
  //https://stackoverflow.com/questions/3915410/how-to-convert-srgb-to-cielab-and-cielab-to-srgb-efficiently
  //this assumes sRGB (r,g,b in [0,1])

  const toLinear = (c) => {
    if (c <= 0.04045) return c / 12.92;
    return ((c + 0.055) / 1.055) ** 2.4;
  }

  const R = toLinear(r);
  const G = toLinear(g);
  const B = toLinear(b);

  const X = R * 0.4124 + G * 0.3576 + B * 0.1805;
  const Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
  const Z = R * 0.0193 + G * 0.1192 + B * 0.9505;

  const delta = 6 / 29;
  const f = (x) => {
    if (x > delta ** 3) return x ** (1 / 3);
    return (1 / 3) * delta ** 2 * x + 4 / 29;
  }

  const Xn = 0.9505;
  const Yn = 1.0000;
  const Zn = 1.0890;

  const LStar = 116 * f(Y / Yn) - 16;
  const aStar = 500 * (f(X / Xn) - f(Y / Yn));
  const bStar = 200 * (f(Y / Yn) - f(Z / Zn));

  return [LStar, aStar, bStar];
}


function colorDist(color1, color2) {
  //break each color into L*a*b* components
  const [L1, a1, b1] = color1;
  const [L2, a2, b2] = color2;

  //CIE76
  const deltaL = L1 - L2;
  const deltaA = a1 - a2;
  const deltaB = b1 - b2;
  return Math.sqrt(deltaL ** 2 + deltaA ** 2 + deltaB ** 2);
}


function encodeImage(width, height, data) {

  //start encoding rawData into 8888 data format
  const pixels = []
  for (let index = 0; index < data.length; index += 4) {
    const a = data[index + 3];
    const r = data[index];
    const g = data[index + 1];
    const b = data[index + 2];

    //encode as ARGB uint32
    let value = ((a << 24) | (r << 16) | (g << 8) | b) >>> 0;

    pixels.push(value)
  }


  //this is a dumb way, but it's O(n), so :D
  const uniqueColors = [...new Set(pixels)];
  const colorFreq = {};
  for (const color of uniqueColors) {
    colorFreq[color] = 0;
  }
  for (const pixel of pixels) {
    colorFreq[pixel]++;
  }

  //sort by frequency
  const sortedColors = Object.keys(colorFreq).sort((a, b) => colorFreq[b] - colorFreq[a]);

  //maximum number of unique colors
  const COLOR_MAX = 255;
  let palette = [];

  //initial distance threshold, will increase later
  let distanceThresh = 9;

  //if we need to squash colors
  if (sortedColors.length > COLOR_MAX) {
    console.log("Too many colors, squashing")

    //create a bool array because array ops are expensive
    //0 = unres 1 = source 2 = linked
    const resolved = new Array(sortedColors.length).fill(0);
    let toResolve = sortedColors.length;
    let seeds = 0;
    const colorMap = {};

    //pre-convert all the colors to L*a*b* colorspace
    const convertedColors = [];
    for (const color of sortedColors) {
      const r = (color & 0x00FF0000) >> 16;
      const g = (color & 0x0000FF00) >> 8;
      const b = color & 0x000000FF;

      const [LStar, aStar, bStar] = RGBtoCIELAB(r / 255, g / 255, b / 255);
      convertedColors.push([LStar, aStar, bStar]);
    }

    let passes = 0;


    while (toResolve > 0 && passes < 100) {

      let marked = 0;

      //do a pass over
      for (let i = 0; i < sortedColors.length; i++) {
        const status = resolved[i];
        if (status == 2) continue;

        const seedColor = sortedColors[i];
        if (status === 0 && seeds < COLOR_MAX) {
          //mark this as a seed
          resolved[i] = 1;
          colorMap[seedColor] = seedColor;
          seeds++;
          toResolve--;
          marked++;
          palette.push(Number(seedColor))
        }

        //looks for similar colors
        for (let j = i + 1; j < sortedColors.length; j++) {
          const status = resolved[j];
          if (status !== 0) continue;

          const testColor = sortedColors[j];
          const dist = colorDist(convertedColors[i], convertedColors[j]);
          if (dist < distanceThresh) {
            //mark as linked
            resolved[j] = 2;
            colorMap[testColor] = seedColor;
            toResolve--;
            marked++;
          }
        }
      }

      //if we still have too many colors, increase the threshold
      console.log("Pass complete, colors left: " + toResolve + " threshold: " + distanceThresh);
      distanceThresh += 0.5;
      passes++;
    }

    //rewrite the pixel array
    for (let i = 0; i < pixels.length; i++) {
      pixels[i] = colorMap[pixels[i]]
    }
  }
  else {
    palette = uniqueColors;
  }

  console.log("Palette size: " + palette.length);

  const paletteMap = {};
  for (let i = 0; i < palette.length; i++) {
    paletteMap[palette[i]] = i;
  }

  const pixelValues = [];
  for (let i = 0; i < pixels.length; i++) {
    pixelValues.push(paletteMap[pixels[i]]);
  }

  const bufSize = 5 + (palette.length * 4) + (pixelValues.length);
  const buf = Buffer.alloc(bufSize);

  //header
  //palette len, width, height
  buf.writeUInt8(palette.length, 0);
  buf.writeUInt16LE(width, 1);
  buf.writeUInt16LE(height, 3);

  //write palette
  for (let i = 0; i < palette.length; i++) {
    const color = palette[i];
    buf.writeUInt32LE(color, 5 + (i * 4));
  }

  //write pixels
  for (let i = 0; i < pixelValues.length; i++) {
    buf.writeUInt8(pixelValues[i], 5 + (palette.length * 4) + i);
  }

  return buf;
}

async function main() {

  for (const file of files) {
    const { path, name } = file;

    if (!name.endsWith(".png")) continue;

    const frameData = await new Promise((resolve, reject) => {
      fs.createReadStream(path + name)
        .pipe(new PNG())
        .on('parsed', function () {
          resolve(this);
        });
    });

    const content = encodeImage(frameData.width, frameData.height, frameData.data);
    fs.writeFileSync(compiledPath + name.replace(".png", ".sprite"), content);
  }
}
main();