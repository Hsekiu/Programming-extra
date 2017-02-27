var canvas = document.getElementById('background');
var context = canvas.getContext('2d');

var cellSize = 30;

var numCells = 20; 

canvas.width = cellSize*numCells;
canvas.height = cellSize*numCells;

var directions = [[-1,-1], [-1,0], [-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];

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

function modMath(x) {
	var res = 0;

	if (x >= 0) {
		res = x%numCells;
	} else {
		res = (x)%numCells + numCells;
	}
	return res;
}

function alive(x, y, cells2d) {
	var count = 0;

	for(var i = 0; i < directions.length; i++) {
		var xd = modMath(directions[i][0] + x);
		var yd = modMath(directions[i][1] + y);

		if(cells2d[xd][yd] == true) {
			count++;
		}
	}
	return count;
}

function step(cells2d) {
	var steparr = drawStage();
	for(var i = 0; i < cells2d.length; i++) {
        for(var j = 0; j < cells2d[i].length; j++) {

        	//Check conditions on live cells
        	if (cells2d[i][j] == true) {
        		//Live cells die with overpopulation and underpopulation
        		if(alive(i,j,cells2d) < 2 || alive(i,j,cells2d) > 3) {
					steparr[i][j] = false;
				//Live cells with 2 or 3 live
        		} else {
        			steparr[i][j] = true;
        		}
			//Check conditions on dead cells
        	} else {
        		if(alive(i,j,cells2d) == 3) {
        			steparr[i][j] = true;
        		}
        	}
        }
    }
    return steparr;
}

var cells2d = [];
cells2d = drawGame(drawStage(cells2d));
cells2d[2][3] = true;
cells2d[3][3] = true;
cells2d[1][3] = true;
cells2d[2][1] = true;
cells2d[3][2] = true;

drawGame(cells2d);
var myStep = setInterval(main, 100);

function main() {
	var steparr = step(cells2d);
	drawGame(steparr);
	cells2d = steparr;
}
