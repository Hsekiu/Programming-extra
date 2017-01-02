var canvas = document.getElementById('background');
var context = canvas.getContext('2d');

var cellSize = 10;

var numCells = 100; 

canvas.width = cellSize*numCells;
canvas.height = cellSize*numCells;

function drawStage() {
	var cells2d = [];
	for (var i = 0; i < numCells; i++) {
		var cells1d = [];
		for (var j = 0; j < numCells; j++) {
			cells1d.push(false);
		}
		cells2d.push(cells1d);
	}
	return cells2d;
}

function drawGame(cells2d) {
	for(var i = 0; i < cells2d.length; i++) {
        for(var j = 0; j < cells2d[i].length; j++) {
            drawCell(i,j,cells2d[i][j]);
        }
    }
    return cells2d;
}

function drawCell(x, y, state) {
	if (state == true) {
		context.fillStyle = 'black';
	} else {
		context.fillStyle = 'grey';
	}
	context.beginPath();
	context.rect(x*cellSize, y*cellSize, cellSize, cellSize);
	context.fill()
}

function step(cells2d) {
	var temp2d = drawGame();
	for(var i = 0; i < cells2d.length-1; i++) {
        for(var j = 0; j < cells2d[i].length-1; j++) {
        	if (cells2d[i][j] == true) {
        		console.log('should trigger once no times');
        		temp2dtest[i][j] = false;
        		temp2dtest[i+1][j] = true;
        	} else {
        		temp2dtest[i][j] = false;
        	}
        }
    }
    return temp2dtest;
}

var cells2d = [];
var temp2d = [];
cells2d = drawGame(drawStage(cells2d));
cells2d[5][5] = true;
drawGame(cells2d);
console.log(cells2d);
temp2d = cells2d;
console.log(temp2d);
var myStep = setInterval(main, 500);

/*function main() {
	console.log('sup');
	temp2d = step(cells2d);
	cells2d = temp2d;
	drawGame(cells2d);
}*/
