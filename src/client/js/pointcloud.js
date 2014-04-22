function PointCloud(minDepth, pointSize, maxBuffer, chSize, verb){
	this.chunk = chSize || 17;
	this.octreeMinDepth = minDepth || 6;
	this.octreeMaxDepth = minDepth || 6;
	this.POINT_SIZE_SCALE = pointSize || 2.5;
	this.point_size = 1;
	this.verbose = verb || false;
	this.initDownloadList = [];
	this.currFileNum = 0;
	this.octreeArr = [];
	this.octreeRootHalfWidth = 0;
	this.totalDrawBufferSize = 0;
	this.currDrawBufferPosition = 0;
	this.maxBuffer = maxBuffer || 3300000;
	this.totalUpdateTime = 0;
	this.maxUpdateTime = 0;
	this.updates = 0;


}

/**
* Initialise pointcloud. Downloads two files sequentially. First is index.oct, to set
* global pointcloud parameters; second is the first data file in the sequence. After this 
* is downloaded it creates the initial geometry, starts the recursive download of all 
* other files in dataset, and calls callback. Callback function MUST initialise liteGL.js!!
* @method initPointCloud
* @param {sourcedir} source url for directory with data files
* @param {callback} function called after first data file is downloaded
*/
PointCloud.prototype.initPointCloud = function(sourcedir, callback) {

	var request = new XMLHttpRequest();
	request.open('GET', sourcedir + 'index.oct', true);
	request.onload = function() {
		if (request.status >= 200 && request.status < 400){
			data = request.responseText;
			var lines = data.split("\n");
			for (var i in lines) {
				if (i == 0) {this.octreeRootHalfWidth = parseInt(lines[i]);continue;}
				if (i == 1) {this.totalDrawBufferSize = parseInt(lines[i]);continue;}
				if (lines[i].length > 0)
					this.initDownloadList.push(sourcedir + lines[i]+".oct");

			}


			var mRequest = new XMLHttpRequest();
			mRequest.open('GET', sourcedir + 'r.oct');
			mRequest.responseType = 'arraybuffer';
			mRequest.onload = function () {
				if (mRequest.status >= 200 && mRequest.status < 400){
					this.octreeArr = this.readOctreeBinFile(mRequest.response);
					if(callback)
						callback();
					this.ready = true;
					this.createGeometry();
					this.startDownload();
				}
			}.bind(this);
			mRequest.send();
		}
	}.bind(this);
	request.send();
}

PointCloud.prototype.startDownload = function() {
	var filename = this.initDownloadList[this.currFileNum];
	//console.log("TODO: add callback for when starting new file");

	if(this.maxBuffer > 3.5e6)
		this.maxBuffer = 3.5e6;

	reqw = new XMLHttpRequest
	reqw.open('GET', filename, true)
	reqw.responseType = 'arraybuffer';
	reqw.onload = function() {
		if (reqw.status >= 200 && reqw.status < 400){

			this.currFileNum++;
			this.updateBinGeometry(reqw.response);

			
			if (this.currFileNum < this.initDownloadList.length && this.octreeArr.length < this.maxBuffer) {
				

				this.startDownload();
			} 
		} 
	}.bind(this);
	reqw.send();
}

PointCloud.prototype.createBinLeaf = function(theDepth, floatArray, intArray, mask) {
	newLeaf = {};
	newLeaf.depth = theDepth;
	
	newLeaf.visible = true;
	newLeaf.position = [floatArray[0], floatArray[1], floatArray[2]];
	if (newLeaf.depth == -1) {
		newLeaf.halfWidth = 0.01;
		newLeaf.color = [intArray[0]/255, intArray[1]/255, intArray[2]/255];
	}
	else {
		newLeaf.halfWidth = this.octreeRootHalfWidth / Math.pow(2, newLeaf.depth);
		newLeaf.color = [intArray[0]/255, intArray[1]/255, intArray[2]/255];
	}
	newLeaf.childMask = mask;
	newLeaf.numChildren = 0;
	for (var i = 0; i < 8; i++) {
		if (mask.charAt(i)=="1")
			newLeaf.numChildren += 1;
	};
	newLeaf.children = [,,,,,,,,];

	return newLeaf;
}

PointCloud.prototype.readOctreeBinFile = function(buffer) {

	objectToReturn = [];

	var dataview = new DataView(buffer);

    var mFloatArray = new Float32Array(3);
    var mIntArray = new Uint8Array(3);
    var mask; var childbitMask; var theDepth;


    for (var row = 0; row < (buffer.byteLength / this.chunk); row ++) {
    	theDepth = dataview.getUint8(row*this.chunk);

        // Copy floats
        for (var i = 0; i < 3; i++) 
            mFloatArray[i] = dataview.getFloat32((row*this.chunk)+1+(i * 4), true);
        for (var i = 0; i < 3; i++) 
            mIntArray[i] = dataview.getInt8((row*this.chunk)+13+i); 
        

        mask = dataview.getUint8((row*this.chunk)+16);
        childbitMask = "";
        for (var i = 0; i < 8; i++){
        	var bob = (mask & 1<<i)
	        if (bob) childbitMask += "1";
	        else childbitMask += "0"
	    }
		var newLeaf = this.createBinLeaf(theDepth, mFloatArray, mIntArray, childbitMask);
		objectToReturn.push(newLeaf);
	}


	return objectToReturn;
}

PointCloud.prototype.countOctreeAtLevel = function(level) {
	var count = 0;
	for (l in octreeObj) {
		if (octreeObj[l].depth == level)
			count++;
	}
	return count;
}

PointCloud.prototype.updateBinGeometry = function(newBuffer) {
	if (this.verbose)
		var start = window.performance.now();

	var buffers = this.mesh.vertexBuffers;
	var newBuffers = {};

	var dataview = new DataView(newBuffer);
    var mFloatArray = new Float32Array(3);
    var mIntArray = new Uint8Array(3);
    var mask; var childbitMask; var theDepth;

	var newBufferLength = (newBuffer.byteLength / this.chunk);

	for (var name in buffers)
	{
		var buffer = buffers[name];
		newBuffers[name] = {};
		newBuffers[name].name = buffer.spacing;
		newBuffers[name].data  = new Float32Array(newBufferLength*buffer.spacing);
	}
	var tempDrawBufferPosition = 0;


	var reloadMesh = false;


	var lineData;
	var newLeaf;
	for (var row = 0; row < (newBuffer.byteLength / this.chunk); row ++) {


  		theDepth = dataview.getInt8(row*this.chunk);

        // Copy floats
        for (var i = 0; i < 3; i++) 
            mFloatArray[i] = dataview.getFloat32((row*this.chunk)+1+(i * 4), true);
        for (var i = 0; i < 3; i++) 
            mIntArray[i] = dataview.getUint8((row*this.chunk)+13+i); 
        

        mask = dataview.getUint8((row*this.chunk)+16);
        childbitMask = "";
        for (var i = 0; i < 8; i++){
        	var bob = (mask & 1<<i)
	        if (bob) childbitMask += "1";
	        else childbitMask += "0"
	    }
		var newLeaf = this.createBinLeaf(theDepth, mFloatArray, mIntArray,childbitMask);
		this.octreeArr.push(newLeaf);


		for (var i = 6; i < 14; i++) {
			if (newLeaf.depth == i && newLeaf.numChildren == 0){
				newBuffers.vertices.data[tempDrawBufferPosition*3] = newLeaf.position[0];
				newBuffers.vertices.data[tempDrawBufferPosition*3+1] = newLeaf.position[1];
				newBuffers.vertices.data[tempDrawBufferPosition*3+2] = newLeaf.position[2];

				newBuffers.colors.data[tempDrawBufferPosition*4] = newLeaf.color[0];
				newBuffers.colors.data[tempDrawBufferPosition*4+1] = newLeaf.color[1];
				newBuffers.colors.data[tempDrawBufferPosition*4+2] = newLeaf.color[2];
				newBuffers.colors.data[tempDrawBufferPosition*4+3] = 1.0;

				// newBuffers.colors.data[tempDrawBufferPosition*4] = depthColors[newLeaf.depth][0];
				// newBuffers.colors.data[tempDrawBufferPosition*4+1] = depthColors[newLeaf.depth][1];
				// newBuffers.colors.data[tempDrawBufferPosition*4+2] = depthColors[newLeaf.depth][2];
				// newBuffers.colors.data[tempDrawBufferPosition*4+3] = 1.0;

				newBuffers.extra.data[tempDrawBufferPosition] = newLeaf.halfWidth*this.POINT_SIZE_SCALE;
				tempDrawBufferPosition++;
			}
		}
		if (newLeaf.depth == -1){
			newBuffers.vertices.data[tempDrawBufferPosition*3] = newLeaf.position[0];
			newBuffers.vertices.data[tempDrawBufferPosition*3+1] = newLeaf.position[1];
			newBuffers.vertices.data[tempDrawBufferPosition*3+2] = newLeaf.position[2];

			newBuffers.colors.data[tempDrawBufferPosition*4] = newLeaf.color[0];
			newBuffers.colors.data[tempDrawBufferPosition*4+1] = newLeaf.color[1];
			newBuffers.colors.data[tempDrawBufferPosition*4+2] = newLeaf.color[2];
			newBuffers.colors.data[tempDrawBufferPosition*4+3] = 1.0;

			newBuffers.extra.data[tempDrawBufferPosition] = newLeaf.halfWidth*this.POINT_SIZE_SCALE;
			tempDrawBufferPosition++;
		}
		if (newLeaf.depth > this.octreeMaxDepth) {
			this.octreeMaxDepth++;
			reloadMesh = true;
		}
	}

	if (reloadMesh){
		this.createGeometry();
		return;
	}


	for (var name in buffers)
	{
		var buffer = buffers[name];
		var newBuffer = newBuffers[name];

		gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);
		gl.bufferSubData(gl.ARRAY_BUFFER, 
		 				  this.currDrawBufferPosition*4*buffer.spacing,
		 				  newBuffer.data);
	}

	this.currDrawBufferPosition+=tempDrawBufferPosition;

	if (this.verbose) {
		var end = window.performance.now();
		var time = end - start;
		this.totalUpdateTime += time;
		this.updates++;
		if (time > this.maxUpdateTime) this.maxUpdateTime = time;
		console.log(time + ". Max: " + this.maxUpdateTime + ". Average: "+ (this.totalUpdateTime/this.updates));
	}
}

PointCloud.prototype.createGeometry = function() {

	var buffers = {};
	buffers.vertices = new Float32Array(this.totalDrawBufferSize*3);
	buffers.colors = new Float32Array(this.totalDrawBufferSize*4);
	buffers.extra = new Float32Array(this.totalDrawBufferSize);
	this.currDrawBufferPosition = 0;
	var currLeaf;

	for (var i = 0; i < this.octreeArr.length; i++) {
		currLeaf = this.octreeArr[i];

		if 	(currLeaf.depth == this.octreeMaxDepth-1 
			 | (currLeaf.depth < this.octreeMaxDepth-1 && currLeaf.depth > this.octreeMinDepth && currLeaf.numChildren == 0) 
			) 
		{


			buffers.vertices[this.currDrawBufferPosition*3] = currLeaf.position[0];
			buffers.vertices[this.currDrawBufferPosition*3+1] = currLeaf.position[1];
			buffers.vertices[this.currDrawBufferPosition*3+2] = currLeaf.position[2];

			// buffers.colors[currDrawBufferPosition*4] = depthColors[currLeaf.depth][0];
			// buffers.colors[currDrawBufferPosition*4+1] = depthColors[currLeaf.depth][1]
			// buffers.colors[currDrawBufferPosition*4+2] = depthColors[currLeaf.depth][2];
			// buffers.colors[currDrawBufferPosition*4+3] = 1.0;

			buffers.colors[this.currDrawBufferPosition*4] = currLeaf.color[0];
			buffers.colors[this.currDrawBufferPosition*4+1] = currLeaf.color[1]
			buffers.colors[this.currDrawBufferPosition*4+2] = currLeaf.color[2];
			buffers.colors[this.currDrawBufferPosition*4+3] = 1.0;

			buffers.extra[this.currDrawBufferPosition] = currLeaf.halfWidth*this.POINT_SIZE_SCALE;
			this.currDrawBufferPosition++;

		}

		if (currLeaf.depth == -1) {
			buffers.vertices[this.currDrawBufferPosition*3] = currLeaf.position[0];
			buffers.vertices[this.currDrawBufferPosition*3+1] = currLeaf.position[1];
			buffers.vertices[this.currDrawBufferPosition*3+2] = currLeaf.position[2];

			buffers.colors[this.currDrawBufferPosition*4] = currLeaf.color[0];
			buffers.colors[this.currDrawBufferPosition*4+1] = currLeaf.color[1]
			buffers.colors[this.currDrawBufferPosition*4+2] = currLeaf.color[2];
			buffers.colors[this.currDrawBufferPosition*4+3] = 1.0;

			buffers.extra[this.currDrawBufferPosition] = currLeaf.halfWidth*this.POINT_SIZE_SCALE;
			this.currDrawBufferPosition++;
		}



	}


	options = {};
	this.mesh = Mesh.load(buffers, options);
}

PointCloud.prototype.draw = function(pointShader, heightOfNearPlane, mvp) {
	pointShader.uniforms({
		u_heightOfNearPlane: heightOfNearPlane,
		u_mvp: mvp,
		u_point_size: this.point_size
	})


	var vertexBuffers = this.mesh.vertexBuffers;
	var indexBuffer = this.mesh.indexBuffers['triangles'];
	var mode = gl.POINTS;


	var length = 0;
	for (var name in vertexBuffers)
	{
		var buffer = vertexBuffers[name];
		var attribute = buffer.attribute || name;
		//precompute attribute locations in shader
		var location = pointShader.attributes[attribute] || gl.getAttribLocation(pointShader.program, attribute);
		 
		if (location == -1 || !buffer.buffer)
		continue; //ignore this buffer
		 
		pointShader.attributes[attribute] = location;
		gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);
		gl.enableVertexAttribArray(location);
		gl.vertexAttribPointer(location, buffer.buffer.spacing, gl.FLOAT, false, 0, 0);
		length = buffer.buffer.length / buffer.buffer.spacing;
	}
	var offset = 0;
	var length = this.currDrawBufferPosition;
	if (length) {
		gl.drawArrays(mode, 0, length);
	}

}