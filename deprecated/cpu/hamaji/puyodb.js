var TBL = [];
for (var i = 0; i < 10; i++) {
  TBL.push('' + i);
}
for (var i = 0; i < 26; i++) {
  TBL.push(String.fromCharCode(i + 97));
}
for (var i = 0; i < 26; i++) {
  TBL.push(String.fromCharCode(i + 65));
}

var g_prev_query = null;
var g_dont_update_url = false;

function $(i) { return document.getElementById(i); }

function indexOfTBL(e) {
  if (TBL.indexOf)
    return TBL.indexOf(e);
  for (var i = 0; i < TBL.length; i++) {
    if (TBL[i] == e)
      return i;
  }
  return null;
}

function strippedQuery() {
  return $('input').value.replace(/[- ]/g, '');
}

function getSmileLink(smile) {
  if (smile['smile'] && smile['smile'].match(/^sm/)) {
    return ('<a href="http://www.nicovideo.jp/watch/' + smile['smile'] +
            '">' + smile.title + '</a> ');
  } else {
    return smile.title + ' ';
  }
}

function normalizeSeq(q) {
  var A = 'A'.charCodeAt(0);
  var D = 'D'.charCodeAt(0);

  var tbl = [0, 0, 0, 0];
  var cur = A;
  for (var i = 0; i < q.length && cur <= D; i += 2) {
    var a = q.charCodeAt(i) - A;
    var b = q.charCodeAt(i+1) - A;
    if (a == b) {
      if (!tbl[a]) {
        tbl[a] = cur++;
      }
    } else {
      if (tbl[a]) {
        if (!tbl[b]) {
          tbl[b] = cur++;
        }
      } else if (tbl[b]) {
        tbl[a] = cur++;
      } else {
        if (a > b) {
          var t = a;
          a = b;
          b = t;
        }

        var a_first = true;
        for (var j = i + 2; j < q.length; j += 2) {
          var x = q.charCodeAt(j) - A;
          var y = q.charCodeAt(j+1) - A;
          if (x > y) {
            var t = x;
            x = y;
            y = t;
          }

          if (a == x && b == y)
            continue;

          if (a == x || a == y) {
            break;
          } else if (b == x || b == y) {
            a_first = false;
            break;
          }
        }

        if (a_first) {
          tbl[a] = cur++;
          tbl[b] = cur++;
        } else {
          tbl[b] = cur++;
          tbl[a] = cur++;
        }
      }
    }
  }

  var nq = '';
  for (var i = 0; i < q.length; i += 2) {
    var a = tbl[q.charCodeAt(i) - A];
    var b = tbl[q.charCodeAt(i+1) - A];
    if (b < a) {
      nq += String.fromCharCode(b);
      nq += String.fromCharCode(a);
    } else {
      nq += String.fromCharCode(a);
      nq += String.fromCharCode(b);
    }
  }

  return nq;
}

function encodeTumo(t) {
  var a = t.charCodeAt(0) - 65;
  var b = t.charCodeAt(1) - 65;
  v = a * 2 + b * 12;
  return TBL[v];
}

function encodeSeq(q) {
  var e = '';
  for (var i = 0; i < q.length; i += 2) {
    e += encodeTumo(q.substr(i, 2));
  }
  return e;
}

function error(msg) {
  $('error').innerHTML = msg;
}

function putPuyo(board, p, x) {
  var y = 0;
  for (; board[x][y]; y++) {}
  board[x][y] = p;
  //console.log(x);
  //console.log(y);
  //console.log(board);
}

function getBoard(seq, dec) {
  var board = [[], [], [], [], [], []];
  //console.log(dec);
  for (var i = 0; i < dec.length; i++) {
    var d = indexOfTBL(dec.charAt(i)) / 2;
    //console.log(d);
    var x = d % 6;
    var r = d / 6 | 0;
    var a = seq.charAt(i*2);
    var b = seq.charAt(i*2+1);
    switch (r) {
    case 0:
      putPuyo(board, a, x);
      putPuyo(board, b, x);
      break;
    case 1:
      putPuyo(board, a, x);
      putPuyo(board, b, x + 1);
      break;
    case 2:
      putPuyo(board, b, x);
      putPuyo(board, a, x);
      break;
    case 3:
      putPuyo(board, a, x);
      putPuyo(board, b, x - 1);
      break;
    default:
      console.error(d);
    }
  }
  return board;
}

function getBoardUrl(board) {
  var q = '';
  for (var y = 12; y >= 0; y--) {
    for (var x = 0; x < 6; x++) {
      if (board[x][y] && board[x][y].match(/[ABCD]/)) {
        q += board[x][y].charCodeAt(0) - 'A'.charCodeAt(0) + 4;
      } else {
        q += 0;
      }
    }
  }
  return 'http://www.inosendo.com/puyo/rensim/?' + q;
}

function getBoardHtml(board, height) {
  if (!height)
    height = 0;
  for (var x = 0; x < 6; x++) {
    if (height < board[x].length)
      height = board[x].length;
  }

  //console.log(board);

  var out = '<table>';
  for (var y = height; y >= 0; y--) {
    out += '<tr><td class="W">■';
    for (var x = 0; x < 6; x++) {
      var p = board[x][y];
      if (p) {
        out += '<td class="' + p + '">' + p + '</span>';
      } else {
        out += '<td>　';
      }
    }
    out += '<td class="W">■</tr>';
  }

  out += '<tr>';
  for (var x = 0; x < 8; x++) {
    out += '<td class="W">■</td>';
  }
  out += '</tr></table>';

  return out;
}

function handleKey(ev) {
  if (ev.keyCode < 48)
    return;

  var rq = strippedQuery();

  if (rq.length == 0 || rq.length % 2 == 1)
    return;

  var q = normalizeSeq(rq);

  if (q.match(/[^ABCD]/)) {
    error('ABCD 以外は無効な入力です');
    return;
  }

  updateURL();

  if (rq != q) {
    error(q + 'として検索します');
    console.log(rq + ' => ' + q);
  } else {
    error('');
  }

  enc = encodeSeq(q);
  if (g_prev_query == enc)
    return;
  g_prev_query = enc;

  console.log(enc);

  var reg = new RegExp('^' + enc);
  var matches = {};
  var keys = [];
  var hits = 0;
  for (var i = 0; i < DB.length; i++) {
    if (!DB[i].match(reg)) {
      continue;
    }

    var toks = DB[i].split(' ');
    var dec = toks[1].substr(0, enc.length);
    if (!matches[dec]) {
      matches[dec] = [];
      keys.push(dec);
    }
    toks.push(i);
    matches[dec].push(toks);
    hits++;
  }

  keys.sort(function(a, b) {
      return matches[b].length - matches[a].length;
    });

  var out = '';
  out += '<p>' + q + ' に';
  if (hits) {
    out += '対して ' + hits + ' のぷよ譜が見つかりました';
  } else {
    out += '一致するぷよ譜はありませんでした';
  }
  out += '<p><table>';
  for (var ki = 0; ki < keys.length; ki++) {
    out += '<tr>';
    var dec = keys[ki];
    var ms = matches[dec];
    out += '<td>';
    out += getBoardHtml(getBoard(q, dec));
    out += '<br>';
    out += '<td>';
    out += ms.length + ' hit' + (ms.length > 1 ? 's' : '') + '<br>';
    for (var i = 0; i < ms.length; i++) {
      var tumos = ms[i][0];
      var decs = ms[i][1];
      var smile = SMILES[ms[i][2]];
      var dbi = ms[i][5];
      var player = smile['player' + ms[i][4]];
      out += '<a href="#" onclick="return c(' + dbi + ')">' + player + '</a> ';
    }
    out += '</tr>';
  }

  if (out.length == 0) {
    out = '無いです';
  }

  $('result').innerHTML = out;
  $('control').innerHTML = '';
}

function updateControl(dbi, turn) {
  turn |= 0;
  updateURL(dbi, turn);

  var match = DB[dbi].split(' ');
  var tumos = match[0];
  var decs = match[1];
  var smile = SMILES[match[2]];
  var match_num = match[3];
  var player = match[4];
  var url = 'http://ips.karou.jp/simu/pe.html?_';
  for (var j = 0; j < tumos.length; j++) {
    url += tumos.charAt(j) + decs.charAt(j);
  }

  var tumo_str = '';
  for (var j = 0; j < tumos.length; j++) {
    var t = indexOfTBL(tumos.charAt(j)) / 2;
    var a = t % 6;
    var b = t / 6 | 0;
    if (j)
      tumo_str += '-';
    tumo_str += "ABCD".charAt(a);
    tumo_str += "ABCD".charAt(b);
  }

  var out = '';
  out += getSmileLink(smile);
  out += match_num + '戦目';
  out += ' (' + player + 'P)<br>';
  out += ('<a href="' + url + '">' + tumo_str + '</a>' +
          ' (<a href="http://ips.karou.jp/simu/">ぷよシミュレータ</a>)');

  out += '<table><tr><td>';

  tumo_str = tumo_str.replace(/-/g, '');

  out += getBoardHtml(getBoard(tumo_str, decs.substr(0, turn)), 12);

  var button = '<button onclick="updateControl(' + dbi + ',';
  out += button + (turn > 0 ? turn - 1 : 0) + ')">←</button>';
  var next_turn = turn + 1;
  if (next_turn > decs.length)
    next_turn--;
  out += button + next_turn + ')">→</button>';

  out += '<td style="vertical-align: middle">　⇒　';

  var board = getBoard(tumo_str, decs);
  out += '<td>';
  out += getBoardHtml(board, 12);

  out += '</td></tr></table>';

  out += '<a href="' + getBoardUrl(board) + '">連鎖シミュレータ</a>';

  $('control').innerHTML = out;
}

function updateURL(dbi, turn) {
  if (g_dont_update_url)
    return;

  var q = normalizeSeq(strippedQuery());
  var href = '#q=' + q;
  if (dbi)
    href += '&dbi=' + dbi;
  if (turn)
    href += '&turn=' + turn;
  location.href = href;
}

function c(dbi) {
  updateControl(dbi, g_prev_query.length);
  return false;
}

function showStats() {
  var out = '<p>' + (DB.length / 2 | 0) + ' 試合*2プレイヤーから検索します';
  out += ' <a href="http://d.hatena.ne.jp/shinichiro_h/20120920#1348152375">';
  out += '説明</a>';

  var cnt_by_player = {};
  var players = [];
  for (var i = 0; i < DB.length; i++) {
    var match = DB[i].split(' ');
    var smile = SMILES[match[2]];
    var player = smile['player' + match[4]];
    if (!cnt_by_player[player]) {
      players.push(player);
      cnt_by_player[player] = 0;
    }
    cnt_by_player[player]++;
  }
  players.sort();

  out += '<ul>';
  for (var i = 0; i < players.length; i++) {
    var p = players[i];
    out += '<li>' + p + ' さん ' + cnt_by_player[p] + ' 試合';
  }
  out += '</ul>';

  var cnt_by_seq = {};
  var seqs = [];
  var cnt = 4096;
  for (var i = 0; i < 4096; i++) {
    var q = '';
    var n = i;
    for (var j = 0; j < 6; j++) {
      q += "ABCD".charAt(n & 3);
      n >>= 2;
    }

    q = normalizeSeq(q);

    if (q.match('D')) {
      cnt--;
    } else {
      if (!cnt_by_seq[q]) {
        seqs.push(q);
        cnt_by_seq[q] = 0;
      }
      cnt_by_seq[q]++;
    }
  }
  seqs.sort();

  out += '<p>初3手<ul>';
  for (var i = 0; i < seqs.length; i++) {
    var q = seqs[i];
    if (q.match(/^AAAA/))
      continue;
    var per = cnt_by_seq[q] / cnt * 10000 | 0;
    per /= 100;
    out += '<li>';
    out += ('<a href="#" onclick="return init(\'q=' + q + '\')">' +
            q + '</a> ' + per + '%');
  }
  out += '</ul>';

  out += '<p>動画ソース<ul>';
  for (var i = 0; i < SMILES.length; i++) {
    var smile = SMILES[i];
    if (smile['title'] && smile['title'].match(/part1$/i)) {
      out += '<li>';
      out += getSmileLink(smile);
    }
  }
  out += '</ul>';

  $('result').innerHTML = out;
}

function getQueryFromUrl() {
  var q = '';
  var url = location.href;
  var hash = url.search('#');
  if (hash >= 0) {
    var q = url.substring(hash + 1);
  }
  return q;
}

function init(q) {
  if (!q) {
    q = getQueryFromUrl();
  }
  console.log('init: ' + q);

  if (q) {
    var params = {};
    var toks = q.split('&');
    for (var i = 0; i < toks.length; i++) {
      var kv = toks[i].split('=');
      params[kv[0]] = unescape(kv[1]);
    }
    if (params['q']) {
      $('input').value = params['q'];
      handleKey();
      if (params['dbi'] && params['turn']) {
        updateControl(params['dbi'], params['turn']);
      }
    }
  }

  if (!$('result').innerHTML) {
    showStats();
  }

  return false;
}

window.onpopstate = function() {
  console.log('onpopstate');
  if (!getQueryFromUrl()) {
    $('result').innerHTML = '';
    $('control').innerHTML = '';
  }
  g_prev_query = null;
  g_dont_update_url = true;
  init();
  g_dont_update_url = false;
}
