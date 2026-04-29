/*
 =====================================================================
   TIC TAC TOE - Node.js Integration Server
   This file connects the HTML frontend to the C++ backend
   
   How it works:
   1. Serves index.html to the browser
   2. Receives moves from the browser via API
   3. Manages game state (since C++ runs once per game)
   4. Sends back results to the browser
 =====================================================================
*/

const http    = require('http');
const fs      = require('fs');
const path    = require('path');
const { exec } = require('child_process');

const PORT = 3000;

// ─────────────────────────────────────────
// Game state (mirrors C++ Game class logic)
// ─────────────────────────────────────────
let board        = Array(9).fill(null);   // 9 cells like array<char,9>
let current      = 'X';                   // currentPlayer
let gameOver     = false;
let moveCount    = 0;
let names        = { X: 'Player 1', O: 'Player 2' };
let scores       = { X: 0, O: 0, D: 0 };
let moveHistory  = [];                    // vector<string> moveHistory

// Winning combinations — same as winCombos in C++
const WIN_COMBOS = [
  [0,1,2], [3,4,5], [6,7,8],  // rows
  [0,3,6], [1,4,7], [2,5,8],  // columns
  [0,4,8], [2,4,6]            // diagonals
];

// ─────────────────────────────────────────
// Helper: Check win — same logic as C++ checkWin()
// ─────────────────────────────────────────
function checkWin(sym) {
  for (const [a, b, c] of WIN_COMBOS) {
    if (board[a] === sym && board[b] === sym && board[c] === sym) {
      return [a, b, c];
    }
  }
  return null;
}

// ─────────────────────────────────────────
// Helper: Check draw — same as C++ checkDraw()
// ─────────────────────────────────────────
function checkDraw() {
  return board.every(cell => cell !== null);
}

// ─────────────────────────────────────────
// Helper: Save result to file — same as C++ saveResult()
// ─────────────────────────────────────────
function saveResult(result) {
  fs.appendFileSync('game_log.txt', result + '\n');
  console.log('Result saved to game_log.txt:', result);
}

// ─────────────────────────────────────────
// Helper: Reset board for new round
// ─────────────────────────────────────────
function resetBoard() {
  board       = Array(9).fill(null);
  current     = 'X';
  gameOver    = false;
  moveCount   = 0;
  moveHistory = [];
}

// ─────────────────────────────────────────
// Helper: Send JSON response
// ─────────────────────────────────────────
function sendJSON(res, statusCode, data) {
  res.writeHead(statusCode, {
    'Content-Type'                : 'application/json',
    'Access-Control-Allow-Origin' : '*',
    'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type'
  });
  res.end(JSON.stringify(data));
}

// ─────────────────────────────────────────
// Helper: Read request body
// ─────────────────────────────────────────
function readBody(req) {
  return new Promise((resolve) => {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try { resolve(JSON.parse(body)); }
      catch { resolve({}); }
    });
  });
}

// ─────────────────────────────────────────
// HTTP Server
// ─────────────────────────────────────────
const server = http.createServer(async (req, res) => {

  // Handle preflight CORS
  if (req.method === 'OPTIONS') {
    res.writeHead(204, {
      'Access-Control-Allow-Origin' : '*',
      'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type'
    });
    res.end();
    return;
  }

  // ── GET / → serve index.html ──────────────────────────────────
  if (req.method === 'GET' && req.url === '/') {
    const filePath = path.join(__dirname, 'index1.html');
    fs.readFile(filePath, (err, data) => {
      if (err) {
        res.writeHead(404);
        res.end('index1.html not found');
        return;
      }
      res.writeHead(200, { 'Content-Type': 'text/html' });
      res.end(data);
    });
    return;
  }

  // ── POST /start → set player names and reset ─────────────────
  if (req.method === 'POST' && req.url === '/start') {
    const body = await readBody(req);
    names.X = body.player1 || 'Player 1';
    names.O = body.player2 || 'Player 2';
    scores  = { X: 0, O: 0, D: 0 };
    resetBoard();

    console.log(`Game started: ${names.X} (X) vs ${names.O} (O)`);

    sendJSON(res, 200, {
      message : 'Game started',
      names,
      scores,
      board,
      current,
      gameOver
    });
    return;
  }

  // ── POST /move → handle a cell click ─────────────────────────
  if (req.method === 'POST' && req.url === '/move') {
    const body = await readBody(req);
    const cellIndex = parseInt(body.cell); // 0 to 8

    // Validate move
    if (gameOver) {
      sendJSON(res, 400, { error: 'Game is already over' });
      return;
    }
    if (isNaN(cellIndex) || cellIndex < 0 || cellIndex > 8) {
      sendJSON(res, 400, { error: 'Invalid cell. Must be 0-8.' });
      return;
    }
    if (board[cellIndex] !== null) {
      sendJSON(res, 400, { error: 'Cell already occupied!' });
      return;
    }

    // Place symbol — like placeSymbol() in C++
    board[cellIndex] = current;
    moveCount++;

    // Save to move history — like push_back() in C++
    moveHistory.push(`${names[current]} (${current}) -> cell ${cellIndex + 1}`);
    console.log(`Move #${moveCount}: ${names[current]} (${current}) -> cell ${cellIndex + 1}`);

    // Check win
    const winCombo = checkWin(current);
    if (winCombo) {
      gameOver = true;
      scores[current]++;
      const result = `${names[current]} won in ${moveCount} moves.`;
      saveResult(result);

      sendJSON(res, 200, {
        board,
        current,
        gameOver,
        winner     : current,
        winnerName : names[current],
        winCombo,
        scores,
        moveHistory,
        message    : `${names[current]} wins!`
      });
      return;
    }

    // Check draw — like checkDraw() in C++
    if (checkDraw()) {
      gameOver = true;
      scores.D++;
      saveResult(`Draw after ${moveCount} moves.`);

      sendJSON(res, 200, {
        board,
        current,
        gameOver,
        draw       : true,
        scores,
        moveHistory,
        message    : "It's a draw!"
      });
      return;
    }

    // Switch player — like switchPlayer() in C++
    current = current === 'X' ? 'O' : 'X';

    sendJSON(res, 200, {
      board,
      current,
      gameOver,
      scores,
      moveHistory,
      message: `${names[current]} (${current})'s turn`
    });
    return;
  }

  // ── POST /newround → reset board only, keep scores ───────────
  if (req.method === 'POST' && req.url === '/newround') {
    resetBoard();
    sendJSON(res, 200, {
      message : 'New round started',
      board,
      current,
      gameOver,
      scores,
      names
    });
    return;
  }

  // ── GET /history → read game_log.txt ─────────────────────────
  if (req.method === 'GET' && req.url === '/history') {
    try {
      const log = fs.readFileSync('game_log.txt', 'utf8');
      sendJSON(res, 200, { history: log.split('\n').filter(Boolean) });
    } catch {
      sendJSON(res, 200, { history: [] });
    }
    return;
  }

  // ── 404 for anything else ─────────────────────────────────────
  res.writeHead(404);
  res.end('Not found');
});

server.listen(PORT, () => {
  console.log('==============================');
  console.log('  TIC TAC TOE SERVER RUNNING  ');
  console.log('==============================');
  console.log(`Open browser: http://localhost:${PORT}`);
  console.log('Press Ctrl+C to stop');
});
