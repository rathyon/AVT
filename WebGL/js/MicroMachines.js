//global THREE
'use strict';

var scene,
	renderer,
    controls;

var tex_loader = new THREE.TextureLoader();

// ----------[ Camera ]----------\\
var camChase = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);
camChase.position.set(0, 50, 50);
camChase.lookAt(new THREE.Vector3(0,0,0));

var camOrtho = new THREE.OrthographicCamera(window.innerWidth/-4, window.innerWidth/4, window.innerHeight/4, window.innerHeight/-4, 0.1, 10000);
camOrtho.position.set(0,50,0);
camOrtho.lookAt(new THREE.Vector3(0,0,0));

var camTop = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);
camTop.position.set(300,200,0);
camTop.lookAt(new THREE.Vector3(0,0,0));

//active camera
var camera = camChase;

// ----------[ Diretional Light ]----------\\
var directional_light = new THREE.DirectionalLight(0xffffff, 1.5);
directional_light.position.set(0, 100, 70);

// ----------[ Point Lights ]----------\\
var light1 = new THREE.PointLight( 0x00ffff, 1.8);
light1.position.set(200, 50, 0);

var light2 = new THREE.PointLight( 0xffff00, 1.8);
light2.position.set(-200, 50, 0);

//wtf is wrong with the spotlight ?!?!
var spotlight = new THREE.SpotLight(0xffffff);
spotlight.position.set(0,5, 5);
spotlight.castShadow = true;

//Set up shadow properties for the spotlight
spotlight.shadow.mapSize.width = 512;  // default
spotlight.shadow.mapSize.height = 512; // default
spotlight.shadow.camera.near = 0.5;       // default
spotlight.shadow.camera.far = 500      // default

var sourcelight = tex_loader.load('js/textures/sourcelight.png');
var hexagon = tex_loader.load('js/textures/hexagon.png');
var ring = tex_loader.load('js/textures/ring.png');
var flareColor = new THREE.Color( 0x00ffff );

// LensFlare( texture, size, distance, blending, color )
//distance - (optional) (0-1) from light source (0 = at light source, 1 = opposite end) 
var lensFlare = new THREE.LensFlare( sourcelight, 100, 0.05, THREE.AdditiveBlending, flareColor );
lensFlare.position.copy( light1.position );
// .add ( texture, size, distance, blending, color )
lensFlare.add(ring, 200, 0.1, THREE.AdditiveBlending, flareColor);
lensFlare.add(hexagon, 50, 0.3, THREE.AdditiveBlending, flareColor);
lensFlare.add(ring, 100, 0.55, THREE.AdditiveBlending, flareColor);
lensFlare.add(sourcelight, 75, 0.7, THREE.AdditiveBlending, flareColor);
lensFlare.add(hexagon, 90, 0.9, THREE.AdditiveBlending, flareColor);

// ----------[ Geometry ]----------\\
var geo_loader = new THREE.OBJLoader();

var floor_geo = new THREE.PlaneGeometry(256,256);

var car_geo = new THREE.BoxGeometry(3,3,3);
//var car_geo = geo_loader.load('js/models/spiked_ball.obj', function(){console.log("obj load started")});

// ----------[ Materials ]----------\\
var bmap = tex_loader.load('js/textures/normal.png');
var map = tex_loader.load('js/textures/stone.png');

var floor_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  new THREE.Color(0xffffff),
  shininess  :  1000,
  bumpMap    :  bmap,
  map        :  map,
});

// THIS MATERIAL WILL ONLY WORK WITH PLANE GEOMETRY
/*
var bumpMat = new THREE.ShaderMaterial({

	uniforms : {
		color : { value : new THREE.Color(0x666666) },
		light : { value : light.position },
		texture : { type: "t", value : tex_loader.load('js/textures/normal.png')} 
	},

	vertexShader : document.getElementById('bumpVertexShader').textContent,

	fragmentShader : document.getElementById('bumpFragmentShader').textContent

});
*/

// Default shader material that uses base color
var shaderMat = new THREE.ShaderMaterial({

	uniforms : {
		color : { value : new THREE.Color(0x666666) },
		light : { value : light1.position }
	},

	vertexShader : document.getElementById('vertexShader').textContent,

	fragmentShader : document.getElementById('fragmentShader').textContent

});

var debugMat = new THREE.MeshBasicMaterial({color: 0xff0000});

// ----------[ Meshes ]----------\\
var floor = new THREE.Mesh(floor_geo, floor_mat);
floor.receiveShadow = true;

var car = new THREE.Mesh(car_geo, floor_mat);

var forward = false;
var backward = false;
var left = false;
var right = false;

// ----------[ Init ]----------\\
var init = function() {
	scene = new THREE.Scene();
	
	//Window
    renderer = new THREE.WebGLRenderer({alpha : true});
    renderer.setClearColor(0x000000, 1.0);
    renderer.setSize(window.innerWidth, window.innerHeight);
	document.body.appendChild( renderer.domElement );
	
	//Cameras
	scene.add(camOrtho);
	scene.add(camTop);

	car.add(camChase);

	window.addEventListener('resize', resize);
	
	//Lights
	scene.add(light1);
	scene.add(light2);
	car.add(spotlight);
	scene.add( lensFlare );

	//scene.add(directional_light);

	//Axis Helper
    scene.add(new THREE.AxisHelper(10));

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
	//movement keys
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
	//camera switching
	else if(event.key == '1'){
		camera = camOrtho;
	}
	else if(event.key == '2'){
		camera = camTop;
	}
	else if(event.key == '3'){
		camera = camChase;
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

	camChase.aspect = window.innerWidth/window.innerHeight;
	camChase.updateProjectionMatrix();

	camOrtho.left = window.innerWidth/-4;
	camOrtho.right = window.innerWidth/4;
	camOrtho.top = window.innerHeight/4;
	camOrtho.bottom = window.innerHeight/-4;
	camOrtho.updateProjectionMatrix();

	camTop.aspect = window.innerWidth/window.innerHeight;
	camTop.updateProjectionMatrix();
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