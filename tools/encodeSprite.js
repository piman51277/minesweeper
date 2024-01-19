//size of color palette
const paletteSize = 1; //0 to 255
const width = 100; //0 to 65535
const height = 100; //0 to 65535

const bufSize = 5 + (paletteSize * 4) + (width * height);
const outBuffer = Buffer.alloc(bufSize);

outBuffer.writeUInt8(paletteSize, 0);

//width
outBuffer.writeUInt16LE(width, 1);

//height
outBuffer.writeUInt16LE(height, 3);

//color palette
const colorsRGB = [0x9300FF00];
for (let i = 0; i < paletteSize; i++) {
  const color = colorsRGB[i] || 0xFF000000;
  outBuffer.writeUInt32LE(color, 5 + (i * 4));
}

//pixels
for (let i = 0; i < width * height; i++) {
  outBuffer.writeUInt8(0, 5 + (paletteSize * 4) + i);
}

const fs = require('fs');
fs.writeFileSync('sprite.bin', outBuffer);