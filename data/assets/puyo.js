function setPuyoColor(elem, colorName) {
    if (!elem)
        return;

    switch (colorName) {
    case 'R':
        elem.style.backgroundPosition = "0px 0px";
        break;
    case 'B':
        elem.style.backgroundPosition = "-32px 0px";
        break;
    case 'Y':
        elem.style.backgroundPosition = "-64px 0px";
        break;
    case 'G':
        elem.style.backgroundPosition = "-96px 0px";
        break;
    case 'P':
        elem.style.backgroundPosition = "-128px 0px";
        break;
    case '@':
        elem.style.backgroundPosition = "-160px 0px";
        break;
    case ' ':
        elem.style.backgroundPosition = "-160px -192px";
        break;
    }
}

function setScore(elem, n) {
    if (!elem)
        return;
    while (elem.firstChild)
        elem.removeChild(elem.firstChild);
    var text = document.createTextNode("" + n);
    elem.appendChild(text);
}

function setOjama(elem, n) {
    if (!elem)
        return;

    while (elem.firstChild)
        elem.removeChild(elem.firstChild);

    function appendOjamaElem(className) {
        var span = document.createElement('span');
        span.className = 'ojama ' + className;
        elem.appendChild(span);
    }

    while (n > 0) {
        if (n >= 400) {
            appendOjamaElem('ojama-400');
            n -= 400;
            continue;
        }
        if (n >= 300) {
            appendOjamaElem('ojama-300');
            n -= 300;
            continue;
        }
        if (n >= 200) {
            appendOjamaElem('ojama-200');
            n -= 200;
            continue;
        }
        if (n >= 30) {
            appendOjamaElem('ojama-30');
            n -= 30;
            continue;
        }
        if (n >= 6) {
            appendOjamaElem('ojama-6');
            n -= 6;
            continue;
        }
        appendOjamaElem('ojama-1');
        n -= 1;
        continue;
    }
}

function setMessage(elem, message) {
    if (!elem || !message || message == "")
        return;

    while (elem.firstChild)
        elem.removeChild(elem.firstChild);

    var text = document.createTextNode(message);
    elem.appendChild(text);
}

function init() {
    var kPW = 32;
    var kPH = 32;

    var kFieldOffsetX = [1, 1 + kPW * 12];
    var kFieldOffsetY = [kPH * 13, kPH * 13];

    function puyoOffsetX(player, x) {
        return kFieldOffsetX[player - 1] + x * kPW;
    }

    function puyoOffsetY(player, y) {
        return kFieldOffsetY[player - 1] - y * kPH;
    }

    function createElem(id, offsetX, offsetY) {
        var span = document.createElement('span');
        span.className = 'puyo';

        span.id = id;
        span.style.position = 'absolute';
        span.style.width = kPW + 'px';
        span.style.height = kPH + 'px';
        span.style.left = offsetX + 'px';
        span.style.top = offsetY + 'px';
        span.style.overflow = 'hidden';

        setPuyoColor(span, 'R');

        return span;
    }

    function createOjamaElem(id, offsetX, offsetY) {
        var div = document.createElement('div');
        div.className = 'ojama-field';

        div.id = id;
        div.style.position = 'absolute';
        div.style.left = offsetX + 'px';
        div.style.top = offsetY + 'px';

        return div;
    }

    // set field puyos
    for (var player = 1; player <= 2; ++player) {
        for (var y = 1; y <= 13; ++y) {
            for (var x = 1; x <= 6; ++x) {
                var id = "puyo-" + player + "-" + y + "-" + x;
                var elem = createElem(id, puyoOffsetX(player, x), puyoOffsetY(player, y));
                document.getElementById('player-fields').appendChild(elem);
            }
        }
    }

    // set next puyos
    var nexts = [
        createElem('next-1-1-1', puyoOffsetX(1, 8),  puyoOffsetY(1,  9)),
        createElem('next-1-1-2', puyoOffsetX(1, 8),  puyoOffsetY(1, 10)),
        createElem('next-1-2-1', puyoOffsetX(1, 9),  puyoOffsetY(1,  8)),
        createElem('next-1-2-2', puyoOffsetX(1, 9),  puyoOffsetY(1,  9)),
        createElem('next-2-1-1', puyoOffsetX(1, 11), puyoOffsetY(1,  9)),
        createElem('next-2-1-2', puyoOffsetX(1, 11), puyoOffsetY(1, 10)),
        createElem('next-2-2-1', puyoOffsetX(1, 10), puyoOffsetY(1,  8)),
        createElem('next-2-2-2', puyoOffsetX(1, 10), puyoOffsetY(1,  9))
    ];
    for (var i = 0; i < nexts.length; ++i) {
        document.getElementById('player-fields').appendChild(nexts[i]);
    }

    // set ojama field
    var ojama1 = createOjamaElem('ojama-1', puyoOffsetX(1, 1), puyoOffsetY(1, 13));
    var ojama2 = createOjamaElem('ojama-2', puyoOffsetX(2, 1), puyoOffsetY(2, 13));
    document.getElementById('player-fields').appendChild(ojama1);
    document.getElementById('player-fields').appendChild(ojama2);

    setInterval(function() {
        loadData();
    }, 10);
}

function loadData() {
    var xhr = new XMLHttpRequest();

    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            var json = eval('(' + xhr.responseText + ')');
            displayGameState(json);
        }
    }

    xhr.open("GET", "/data", true);
    xhr.send();
}

function displayGameState(json) {
    // p1
    for (var i = 0; i < json.p1.length; ++i) {
        var x = i % 6 + 1;
        var y = 14 - (i - x + 1) / 6;
        var id = "puyo-1-" + y + "-" + x;
        setPuyoColor(document.getElementById(id), json.p1[i]);
    }
    // p1 next
    setPuyoColor(document.getElementById('next-1-1-1'), json.n1[2]);
    setPuyoColor(document.getElementById('next-1-1-2'), json.n1[3]);
    setPuyoColor(document.getElementById('next-1-2-1'), json.n1[4]);
    setPuyoColor(document.getElementById('next-1-2-2'), json.n1[5]);
    // p1 score
    setScore(document.getElementById('score-1'), json.s1);
    // p1 ojama
    setOjama(document.getElementById('ojama-1'), json.o1);
    // p1 set message
    setMessage(document.getElementById('player1-message'), json.m1);

    // p2
    for (var i = 0; i < json.p2.length; ++i) {
        var x = i % 6 + 1;
        var y = 14 - (i - x + 1) / 6;
        var id = "puyo-2-" + y + "-" + x;
        setPuyoColor(document.getElementById(id), json.p2[i]);
    }
    // p2 next
    setPuyoColor(document.getElementById('next-2-1-1'), json.n2[2]);
    setPuyoColor(document.getElementById('next-2-1-2'), json.n2[3]);
    setPuyoColor(document.getElementById('next-2-2-1'), json.n2[4]);
    setPuyoColor(document.getElementById('next-2-2-2'), json.n2[5]);
    // p2 score
    setScore(document.getElementById('score-2'), json.s2);
    // p2 ojama
    setOjama(document.getElementById('ojama-2'), json.o2);
    // p2 set message
    setMessage(document.getElementById('player2-message'), json.m2);
}
