const { Worker, isMainThread, parentPort, workerData } = require('worker_threads');

if (!isMainThread) {

  class MinesweeperGame {
    constructor(rows, cols, mines) {
      this.rows = rows;
      this.cols = cols;
      this.mines = mines;
      this.gameOver = false;
      this.board = this.generateBoard();
      this.revealed = Array.from({ length: rows }, () => Array.from({ length: cols }, () => false));
    }

    generateBoard() {
      let board = new Uint8Array(this.rows * this.cols);
      let mines = this.mines;
      while (mines > 0) {
        let row = Math.floor(Math.random() * this.rows);
        let col = Math.floor(Math.random() * this.cols);
        if (board[row * this.cols + col] === 0) {
          board[row * this.cols + col] = 9;
          mines--;
        }
      }

      // Fill in the numbers
      for (let y = 0; y < this.rows; y++) {
        for (let x = 0; x < this.cols; x++) {
          if (board[y * this.cols + x] === 9) {
            for (let i = -1; i <= 1; i++) {
              for (let j = -1; j <= 1; j++) {
                if (y + i >= 0 && y + i < this.rows && x + j >= 0 && x + j < this.cols) {
                  if (board[(y + i) * this.cols + x + j] !== 9) {
                    board[(y + i) * this.cols + x + j]++;
                  }
                }
              }
            }
          }
        }
      }

      return board;
    }

    printBoard(normalMode = true) {
      let board = this.board;
      let rows = this.rows;
      let cols = this.cols;
      let str = '';
      for (let i = 0; i < rows; i++) {
        for (let j = 0; j < cols; j++) {
          if (!this.revealed[i][j] && normalMode) {
            str += '?';
            continue;
          }

          if (board[i * cols + j] === 9) {
            str += 'X';
          } else {
            str += board[i * cols + j];
          }
        }
        str += '\n';
      }
      console.log(str);
      console.log(`Game over? ${this.gameOver}`);
    }

    reveal(x, y) {
      if (this.gameOver) {
        return;
      }

      if (this.board[y * this.cols + x] === 9) {
        this.gameOver = true;
        this.revealed[y][x] = true;
        this.printBoard();
        return;
      }

      this.revealed[y][x] = true;

      if (this.board[y * this.cols + x] === 0) {
        for (let i = -1; i <= 1; i++) {
          for (let j = -1; j <= 1; j++) {
            if (y + i >= 0 && y + i < this.rows && x + j >= 0 && x + j < this.cols) {
              if (!this.revealed[y + i][x + j]) {
                this.reveal(x + j, y + i);
              }
            }
          }
        }
      }
    }

    reset() {
      this.gameOver = false;
      this.board = this.generateBoard();
      this.revealed = Array.from({ length: this.rows }, () => Array.from({ length: this.cols }, () => false));
    }

    isDone() {
      for (let i = 0; i < this.rows; i++) {
        for (let j = 0; j < this.cols; j++) {
          if (!this.revealed[i][j] && this.board[i * this.cols + j] !== 9) {
            return false;
          }
        }
      }
      return true;
    }

    numRevealed() {
      let count = 0;
      for (let i = 0; i < this.rows; i++) {
        for (let j = 0; j < this.cols; j++) {
          if (this.revealed[i][j]) {
            count++;
          }
        }
      }
      return count;
    }
  }

  function solver(mGame) {
    const { rows, cols, board, revealed } = mGame;
    let changed = false;

    //these are number tiles that are revealed
    //that border a tile that is not revealed
    const frontier = [];

    for (let i = 0; i < rows; i++) {
      for (let j = 0; j < cols; j++) {
        if (revealed[i][j]) {
          for (let x = -1; x <= 1; x++) {
            for (let y = -1; y <= 1; y++) {
              if (i + x >= 0 && i + x < rows && j + y >= 0 && j + y < cols) {
                if (!revealed[i + x][j + y]) {
                  frontier.push([i, j]);
                }
              }
            }
          }
        }
      }
    }

    //for each tile in the frontier
    for (let [i, j] of frontier) {
      //the number of revealed mines around it
      let revealedMines = 0;
      //the number of hidden tiles around it
      let hiddenTiles = 0;

      for (let x = -1; x <= 1; x++) {
        for (let y = -1; y <= 1; y++) {
          if (i + x >= 0 && i + x < rows && j + y >= 0 && j + y < cols) {
            if (revealed[i + x][j + y] && board[(i + x) * cols + j + y] === 9) {
              revealedMines++;
            }

            if (!revealed[i + x][j + y]) {
              hiddenTiles++;
            }
          }
        }
      }


      //if these add up to the number on the tile
      //then we know that all the hidden tiles are mines
      if (revealedMines + hiddenTiles === board[i * cols + j]) {
        for (let x = -1; x <= 1; x++) {
          for (let y = -1; y <= 1; y++) {
            if (i + x >= 0 && i + x < rows && j + y >= 0 && j + y < cols) {
              if (!revealed[i + x][j + y]) {
                revealed[i + x][j + y] = true;
                changed = true;
              }
            }
          }
        }
      }

      //if the number of revealed mines is equal to the number on the tile
      //then we know that all the hidden tiles are safe
      if (revealedMines === board[i * cols + j]) {
        for (let x = -1; x <= 1; x++) {
          for (let y = -1; y <= 1; y++) {
            if (i + x >= 0 && i + x < rows && j + y >= 0 && j + y < cols) {
              if (!revealed[i + x][j + y]) {
                mGame.reveal(j + y, i + x);
                changed = true;
              }
            }
          }
        }
      }
    }

    //TODO: Add 1-1 and 1-2 pattern solvers for more interesting boards

    return changed;
  }

  function generateBoard() {

    const game = new MinesweeperGame(13, 30, 80);
    let initialGuess = [0, 0]
    while (true) {
      game.reset();

      //find a random safe spot
      let x, y;
      while (true) {
        x = Math.floor(Math.random() * 30);
        y = Math.floor(Math.random() * 13);
        if (game.board[y * 30 + x] == 0) {
          break;
        }

      }
      initialGuess = [x, y];
      game.reveal(x, y);

      //we can't reveal more than 40% of the board on the first move
      if (game.numRevealed() > (13 * 30 * 0.4)) {
        continue;
      }

      while (solver(game)) {
        continue;
      }

      if (game.isDone()) {
        break;
      }
    }

    //reset only the revealed tiles
    game.revealed = Array.from({ length: 13 }, () => Array.from({ length: 30 }, () => false));
    game.reveal(...initialGuess);

    //encode the board into the correct format
    //195 bytes
    let buf = Buffer.alloc(195);

    //for every pair of positions
    for (let i = 0; i < 30 * 13; i += 2) {
      //get the board values
      const b1 = game.board[i];
      const b2 = game.board[i + 1];

      const b1Good = game.revealed[Math.floor(i / 30)][i % 30];
      const b2Good = game.revealed[Math.floor((i + 1) / 30)][(i + 1) % 30];

      /**
       * 0 -> generic space
       * 1 -> mine
       * 2 -> ok starting spot (is a 0)
       */

      const b1Code = b1 === 9 ? 1 : ((b1 == 0 && b1Good) ? 2 : 0);
      const b2Code = b2 === 9 ? 1 : ((b2 == 0 && b2Good) ? 2 : 0);

      //encode them in a uint8
      let val = ((b1Code << 4 | b2Code) >> 0) & 0xff;
      //write them to the buffer
      buf.writeUInt8(val, i / 2);
    }

    return buf;
  }

  while (true) {
    const buf = generateBoard();
    const hash = require('crypto').createHash('sha256').update(buf).digest('hex');
    parentPort.postMessage({ type: 'board', board: buf, hash });
  }

} else {
  const threadCount = process.env.THREAD_COUNT || (require('os').cpus().length - 2);
  console.log(`Using ${threadCount} threads`);

  const workers = [];
  for (let i = 0; i < threadCount; i++) {
    workers.push(new Worker(__filename));
  }

  const target = 1000;

  const boards = [];
  const seen = new Set();

  workers.forEach(worker => {
    worker.on('message', (msg) => {
      if (msg.type === 'board') {
        if (!seen.has(msg.hash)) {
          seen.add(msg.hash);
          boards.push(msg.board);

          process.stdout.write(`Progress: ${boards.length}/${target}     `);
          process.stdout.cursorTo(0);

          if (boards.length === target) {
            console.log();
            finish();
          }
        }
      }
    });
  });

  for (let i = 0; i < threadCount; i++) {
    workers[i].postMessage({ type: 'generate' });
  }

  function finish() {
    workers.forEach(worker => worker.terminate());

    const fs = require('fs');
    const boardBuf = Buffer.concat(boards);

    //add the number of boards to the start of the file
    const numBoards = Buffer.alloc(2);
    numBoards.writeUInt16LE(boards.length);

    fs.writeFileSync('assets/compiled/boards.bin', Buffer.concat([numBoards, boardBuf]));
    console.log('Done');
  }
}