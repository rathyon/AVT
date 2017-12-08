//global THREE
'use strict';

var scene,
	renderer,
    controls;

// ----------[ Camera ]----------\\
var camera = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);

// ----------[ Diretional Light ]----------\\
var directional_light = new THREE.DirectionalLight(0xffffff, 1.5);
directional_light.position.set(0, 100, 70);

// ----------[ Point Lights ]----------\\
var light = new THREE.PointLight( 0xffffff, 2.5);
light.position.y = 200;
light.position.x = 200;

// ----------[ Geometry ]----------\\
var geo_loader = new THREE.OBJLoader();

var floor_geo = new THREE.PlaneGeometry(256,256);

var car_geo = new THREE.BoxGeometry(3,3,3);
//var car_geo = geo_loader.load('js/models/spiked_ball.obj', function(){console.log("obj load started")});

// ----------[ Materials ]----------\\
var tex_loader = new THREE.TextureLoader();

var bmap = tex_loader.load('js/textures/normal.png');
var map = tex_loader.load('js/textures/stone.png');

var floor_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  new THREE.Color(0xffffff),
  shininess  :  1000,
  bumpMap    :  bmap,
  map        :  map,
});

var shader_mat = new THREE.ShaderMaterial({

	uniforms : {
		color : { value : new THREE.Color(0x666666) }
	},

	vertexShader : document.getElementById('vertexShader').textContent,

	fragmentShader : document.getElementById('fragmentShader').textContent

});

var debugMat = new THREE.MeshBasicMaterial({color: 0xff0000});

// ----------[ Meshes ]----------\\
var floor = new THREE.Mesh(floor_geo, floor_mat);

var car = new THREE.Mesh(car_geo, shader_mat);

var forward = false;
var backward = false;
var left = false;
var right = false;

// ----------[ Init ]----------\\
var init = function() {
	scene = new THREE.Scene();
	
	//Window
    renderer = new THREE.WebGLRenderer();
    renderer.setSize(window.innerWidth, window.innerHeight);
	document.body.appendChild( renderer.domElement );
	
	//Camera
	car.add(camera);
	camera.position.set(0, 50, 50);
	camera.lookAt(new THREE.Vector3(0,0,0));

	window.addEventListener('resize', resize);
	
	//Point light
	//scene.add(light);

	//Axis Helper
    scene.add(new THREE.AxisHelper(10));
	
	scene.add(directional_light);

	//Objects setup
    scene.add(floor);
    //lay down horizontally
    floor.rotateOnAxis(new THREE.Vector3(1,0,0), THREE.Math.degToRad(-90));

    scene.add(car);
    car.position.y = 1.5;

	//Mouse Controls
    controls = new THREE.OrbitControls( camera, renderer.domElement );
    controls.addEventListener('change', render);

    //Key Down
    document.addEventListener('keydown', keyDown);
    document.addEventListener('keyup', keyUp);

	renderScene();

};

var keyDown = function (event) {
	if(event.key == 'w'){
		forward = true;
	}
	else if(event.key == 'a'){
		left = true;
	}
	else if(event.key == 's'){
		backward = true;
	}
	else if(event.key == 'd'){
		right = true;
	}
};

var keyUp = function (event) {
	if(event.key == 'w'){
		forward = false;
	}
	else if(event.key == 'a'){
		left = false;
	}
	else if(event.key == 's'){
		backward = false;
	}
	else if(event.key == 'd'){
		right = false;
	}
}

var animate = function(){
	if(forward){
		car.position.z += 0.5;
	}
	if(backward){
		car.position.z -= 0.5;
	}
	if(left){
		car.rotateY(THREE.Math.degToRad(1));
	}
	if(right){
		car.rotateY(THREE.Math.degToRad(-1));
	}
};

var resize = function() {
	renderer.setSize(window.innerWidth, window.innerHeight);
	camera.aspect = window.innerWidth/window.innerHeight;
	camera.updateProjectionMatrix();
};

var render = function (){
		renderer.render(scene, camera);
	};

var renderScene = function() {

	requestAnimationFrame(renderScene);
    controls.update();
    animate();
	render();
};