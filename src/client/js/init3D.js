//cam + movement variables
var cam_pos = [0, 0, 0]; 
var cam_pitch = -11;
var cam_yaw = 86;
var cam_forward = [0,0,1];
var move = [false,false,false,false];
var MOVE_SPEED = 20.0;
var heightOfNearPlane = 0;

// pointcloud 
var cloud;


function init(){


	SOURCE_DIR = "../testdatasets/CCSR1/25000/";
	//data will need to be stored on same server to prevent Access-Control-Allow-Origin errors
	//a test dataset can be found at 
	//alunevans.info/apps/webgl/pointclouds/testdatasets/progpointstestdata.zip (66MB zip)

	cam_pos = [-12, -17, -12.5]; 
	cam_pitch = -44.8; cam_yaw = 240.4;

	cloud = new PointCloud(6, 2.5, 3500000);
	cloud.initPointCloud(SOURCE_DIR, initGL);

}

function initGL(stations)
{
	//init liteGL
	var gl = GL.create({width:document.documentElement.clientWidth,height:document.documentElement.clientHeight});
	gl.animate();

	var container = document.querySelector("#content");
	container.appendChild(gl.canvas);


	// init shader
	var pointShader = new Shader(document.getElementById( 'pointVertShader' ).textContent, 
							document.getElementById( 'pointFragShader' ).textContent);


	//create basic matrices for cameras and transformation
	var persp = mat4.create();
	var view = mat4.create();
	var model = mat4.create();
	var mvp = mat4.create();
	var mv = mat4.create();
	mat4.perspective(persp, 45 * DEG2RAD, gl.canvas.width / gl.canvas.height, 0.1, 10000);

	//set camera
	var vAxis = vec3.create();
	var matRot1 = mat4.create();
	var matRot2 = mat4.create();
	var cam_target = [0,0,0];
	heightOfNearPlane = Math.abs(gl.canvas.height/(2*Math.tan(0.5*45.0*Math.PI/180.0)));
	updateCamera();


	//generic gl flags and settings
	gl.clearColor(0.0,0.0,0.0,1);
	gl.enable( gl.DEPTH_TEST );
	gl.enable( gl.CULL_FACE);
	gl.cullFace(gl.BACK);

	
	function updateCamera()
	{
		cam_forward = [0,0, -1];
		//first transformation
		mat4.rotateY(matRot1, mat4.create(), cam_yaw*DEG2RAD);
		vec3.transformMat4(cam_forward, cam_forward, matRot1);
		//get new axis
		vec3.transformMat4(vAxis, [1,0,0],matRot1);
		//get second rotation around new axis
		mat4.rotate(matRot2, mat4.create(), cam_pitch*DEG2RAD, vAxis) ;
		vec3.transformMat4(cam_forward, cam_forward, matRot2);
	}
	
	//rendering loop
	gl.ondraw = function()
	{
		gl.viewport(0,0,gl.canvas.width, gl.canvas.height);
		gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);

		//camera
		vec3.normalize(cam_forward, cam_forward);
		vec3.add(cam_target, cam_pos, cam_forward);
		mat4.lookAt(view, cam_pos, cam_target, [0,1,0]);

		//draw
		mat4.multiply(mv,view,model);
		mat4.multiply(mvp,persp,mv);
		cloud.draw(pointShader, heightOfNearPlane, mvp);
	};

	
	gl.onupdate = function(dt) {

		stats.update();

		if (move[0]) {
			var tmp = vec3.create();
			vec3.add(tmp, cam_pos,cam_forward);
			vec3.lerp(cam_pos, cam_pos, tmp, dt*MOVE_SPEED);
		} else if (move[1]) {
			var tmp = vec3.create();
			vec3.subtract(tmp, cam_pos,cam_forward);
			vec3.lerp(cam_pos, cam_pos, tmp, dt*MOVE_SPEED);
		} else if (move[2]) {
			var xp = vec3.create();
			vec3.cross(xp, cam_forward, [0,1,0]);
			var tmp = vec3.create();
			vec3.subtract(tmp, cam_pos,xp);
			vec3.lerp(cam_pos, cam_pos, tmp, dt*MOVE_SPEED);			
		} else if (move[3]) {
			var xp = vec3.create();
			vec3.cross(xp, cam_forward, [0,1,0]);
			var tmp = vec3.create();
			vec3.add(tmp, cam_pos,xp);
			vec3.lerp(cam_pos, cam_pos, tmp, dt*MOVE_SPEED);	
		}
		

	};

	////////////////////////////////////////////////////////////////
	// Get Mouse
	////////////////////////////////////////////////////////////////
	gl.captureMouse();
	gl.onmousemove = function(e)
	{
		if(e.dragging) {
			cam_pitch -= e.deltay*0.5;
			cam_yaw -= e.deltax*0.5;
			if (cam_pitch > 90) cam_pitch = 90;
			if (cam_pitch < -90) cam_pitch = -90;
			if (cam_yaw > 90) cam_yaw -= 360;
			if (cam_yaw < 0) cam_yaw += 360;

			updateCamera();
		}
	}

	gl.captureKeys(true);
	gl.onkeydown = function(e) { 

		if (e.keyCode == 87) {
			move[0] = true;
		} else if (e.keyCode == 83) {
			move[1] = true;
		} else if (e.keyCode == 65) {
			move[2] = true;
		} else if (e.keyCode == 68) {
			move[3] = true;
		} 
	}
	gl.onkeyup = function(e) { 
		if (e.keyCode == 87) {
			move[0] = false;
		} else if (e.keyCode == 83) {
			move[1] = false;
		} else if (e.keyCode == 65) {
			move[2] = false;
		} else if (e.keyCode == 68) {
			move[3] = false;
		}	
	}
}	