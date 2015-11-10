const EMPTY = 0;
const RED = 1;
const BLUE = 2;
const YELLOW = 3;
const GREEN = 4;
const PURPLE = 5;
const OJAMA = 6;

function colorNames(color) {
    return ['empty', 'red', 'blue', 'yellow', 'green', 'purple', 'ojama'][color];
}

function colorChar(color) {
    return ['.', 'R', 'B', 'Y', 'G', 'P', 'O'][color];
}

function Field() {
    this.board = [
        //   0      1      2      3      4      5      6      7      8      9     10     11     12     13
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
        [EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY],
    ];
}

Field.prototype = {
    setColor: function(x, y, color) {
        if (x < 1 || 6 < x)
            return;
        if (y < 1 || 13 < y)
            return;

        this.board[x][y] = color;
        document.getElementById('puyo-' + x + '-' + y).className = 'puyo ' + colorNames(color);
    },

    submit: function() {
        var s = "";
        for (var y = 13; y >= 1; --y) {
            for (var x = 1; x <= 6; ++x) {
                s += colorChar(this.board[x][y]);
            }
        }

        console.log(s);

        $.ajax({
            type: 'POST',
            url: '/api/post',
            data: {
                field: s
            }
        }).done(function (data) {
            console.log('DONE');
        });
    }
};

function ColorChooser() {
    this.currentColor = EMPTY;
}

ColorChooser.prototype = {
};

var field = new Field();
var chooser = new ColorChooser();

// ----------------------------------------------------------------------

function init() {
    var fieldArea = document.getElementById('field-area');
    for (var y = 0; y < 14; ++y) {
        for (var x = 0; x < 8; ++x) {
            var span = document.createElement('span');
            span.style.position = 'absolute';
            span.style.width = '32px';
            span.style.height = '32px';
            span.style.left = (x * 32) + 'px';
            span.style.top = ((13 - y) * 32) + 'px';
            span.className = 'puyo';
            span.id = 'puyo-' + x + '-' + y;

            span.onclick = (function(x, y) {
                return function() {
                    field.setColor(x, y, chooser.currentColor);
                };
            })(x, y);
            fieldArea.appendChild(span);
        }
    }
}
