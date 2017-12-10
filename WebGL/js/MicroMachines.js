//global THREE
'use strict';

var scene,
	renderer,
    controls;

var tex_loader = new THREE.TextureLoader();

// ----------[ Consts ]----------\\

const NUMBER_INNER_CHEERIOS = 20;
const NUMBER_OUTER_CHEERIOS = 20;
const NUMBER_CHEERIOS = NUMBER_INNER_CHEERIOS + NUMBER_OUTER_CHEERIOS;

const NUMBER_ORANGES = 5;

const axisX = new THREE.Vector3(1, 0, 0);
const axisY = new THREE.Vector3(0, 1, 0);
const axisZ = new THREE.Vector3(0, 0, 1);

// ----------[ Camera ]----------\\

var camChase = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);
camChase.position.set(0, 50, 50);
camChase.lookAt(new THREE.Vector3(0,0,0));

var camOrtho = new THREE.OrthographicCamera(window.innerWidth/-4, window.innerWidth/4, window.innerHeight/4, window.innerHeight/-4, 0.1, 10000);
camOrtho.position.set(0,1,0);
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

// ----------[ Lens Flare ]----------\\

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

var floor_geo = new THREE.PlaneGeometry(100, 100);

var car_geo = new THREE.BoxGeometry(1, 1, 1);

var cheerio_geo = new THREE.TorusGeometry( 1, 0.5, 16, 60 );

var orange_geo = new THREE.SphereGeometry ( 1, 30, 30 );
//var car_geo = geo_loader.load('js/models/spiked_ball.obj', function(){console.log("obj load started")});


// ----------[ Materials ]----------\\

var bmap = tex_loader.load('js/textures/normal.png');
var map = tex_loader.load('js/textures/stone.png');

var floor_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  0xffffff,
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

var cheerio_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  0xffffff,
  shininess  :  1000,
});

var orange_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  0xffffff,
  shininess  :  1000,
});

var debugMat = new THREE.MeshBasicMaterial({color: 0xff0000});

// ----------[ Meshes ]----------\\

var floor = new THREE.Mesh(floor_geo, floor_mat);
floor.receiveShadow = true;

var car = new THREE.Mesh(car_geo, floor_mat);

// ----------[ Cheerios ]----------\\

var cheerio = [ ];
var cheerioSpeed = [ ];
var cheerioDir = [ ];

for(var i = 0; i< NUMBER_CHEERIOS ; i++) {
    cheerioDir[i] = new THREE.Vector3();
}

const cheerioTraction = 0.003;

// ----------[ Car ]----------\\

const carInitPos = new THREE.Vector3(30, 0.5, 0);
car.position.set(carInitPos.x, carInitPos.y, carInitPos.z);


var carSpeed = 0;
var carAngle = 0;
var carWheelRotation = 0;
const carAcc = 0.002;
const carTraction = 0.001;
const carBrake = 0.005;
const carReverse = 0.001;
const carAngularSpeed = THREE.Math.degToRad(1.5);
const speedLimit = 0.3;

var carForward = false;
var carBackward = false;
var carLeft = false;
var carRight = false;

var wheelTurn = 0;

// ----------[ Oranges ]----------\\

var orange = [ ];
var orangeSpeed = [ ];
var orangeAcceleration = [ ];

function getRandomFloat(min, max) {
  return Math.random() * (max - min) + min;
}




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
    scene.add(new THREE.AxesHelper(10));

	//Objects setup
    scene.add(floor);
    //lay down horizontally
    floor.rotateX(THREE.Math.degToRad(-90));

    scene.add(car);
	
	resetCheerios();
	resetOranges();

	//Mouse Controls
    controls = new THREE.OrbitControls( camera, renderer.domElement );
    controls.addEventListener('change', render);

    //Key Down
    document.addEventListener('keydown', keyDown);
    document.addEventListener('keyup', keyUp);

	renderScene();

};

function resetCheerios() {
	var innerStep = 360 / NUMBER_INNER_CHEERIOS;
	for (var i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		cheerio[i] = new THREE.Mesh(cheerio_geo, cheerio_mat);
		cheerio[i].rotateY(i*innerStep);
		cheerio[i].translateX(20);
		cheerio[i].rotateX(Math.PI / 2);
		cheerio[i].position.y += 0.5;
		scene.add(cheerio[i]);
	}

	var outerStep = 360 / NUMBER_OUTER_CHEERIOS;
	for (var i = NUMBER_INNER_CHEERIOS; i < NUMBER_CHEERIOS; i++) {
		cheerio[i] = new THREE.Mesh(cheerio_geo, cheerio_mat);
		cheerio[i].rotateY(i*outerStep);
		cheerio[i].translateX(40);
		cheerio[i].rotateX(Math.PI / 2);
		cheerio[i].position.y += 0.5;
		scene.add(cheerio[i]);
	}
	
}

function resetOranges() {
	for (var i = 0; i < NUMBER_ORANGES; i++) {
		orange[i] = new THREE.Mesh(orange_geo, orange_mat);
		orange[i].position.set(-50, 1, -30+(15*i));
		scene.add(orange[i]);

		orangeSpeed[i] = getRandomFloat(0.15, 0.45);
		orangeAcceleration[i] = getRandomFloat(0.001, 0.007);
	}
}

function detectCollision (obj1, obj2) {
	var boundingBoxObj1 = new THREE.Box3().setFromObject(obj1);
	var boundingBoxObj2 = new THREE.Box3().setFromObject(obj2);
	
	return boundingBoxObj1.intersectsBox(boundingBoxObj2);
}

function animateCheerios() {
	var aux = new THREE.Vector3();
	for (var i = 0; i < NUMBER_CHEERIOS; i++) {

		if (detectCollision(car, cheerio[i])) {

			if (carSpeed >= 0) {
				cheerioSpeed[i] = carSpeed;
				cheerioDir[i].setX(Math.cos(-carAngle));
				cheerioDir[i].setZ(Math.sin(-carAngle));
			}

			else {
				cheerioSpeed[i] = -carSpeed;
				cheerioDir[i].setX(-Math.cos(-carAngle));
				cheerioDir[i].setZ(-Math.sin(-carAngle));
			}

			carSpeed = 0;
		}

		if (cheerioSpeed[i] > cheerioTraction) {
			cheerioSpeed[i] -= cheerioTraction;
			aux.setX(cheerioDir[i].x * cheerioSpeed[i]);
			aux.setZ(cheerioDir[i].z * cheerioSpeed[i]);
			cheerio[i].position.add(aux);
		}

		else
			cheerioSpeed[i] = 0;
	}
}

function animateOranges() {

	for (var i = 0; i < NUMBER_ORANGES; i++) {


		if (orange[i].position.x >= 100) { // respawn
			orange[i].position.x = -50;
			orange[i].position.y = 1;
			orangeSpeed[i] = getRandomFloat(0.1, 0.4);
			orangeAcceleration[i] = getRandomFloat(0.001, 0.005);
		}
		
		else if (orange[i].position.x >= 50) { // hide orange

			if(orange[i].position.y != -500) // just to avoid too many writes
				orange[i].position.y = -500;

			orange[i].position.x += orangeSpeed[i];
		}
		
		else { // normal movement
			orangeSpeed[i] += orangeAcceleration[i];
			orange[i].position.x += orangeSpeed[i];
			orange[i].rotation.z += orangeSpeed[i];
			orange[i].rotation.z = orange[i].rotation.z % (Math.PI * 2);
		}

	}

}

function animateCar() {
	var aux = new THREE.Vector3();
/*
	if (car.position.x < -50 || car.position.x > 50 || car.position.z > 50 || car.position.z < -50)  // lose life if car is out of bounds
		carDie();*/

	if (carForward) {
		if (carSpeed >= 0)
			carSpeed += carAcc;
		else
			carSpeed += carBrake;

	}

	else if (carBackward) {
		if (carSpeed <= 0)
			carSpeed -= carReverse;
		else
			carSpeed -= carBrake;
	}

	else if (!carBackward && !carForward) {
		if (carSpeed <= carTraction && carSpeed >= -carTraction)
			carSpeed = 0;
		else if (carSpeed > 0)
			carSpeed -= carTraction;
		else
			carSpeed += carTraction;
	}

	if (carSpeed > speedLimit)
		carSpeed = speedLimit;
	else if (carSpeed < -speedLimit)
		carSpeed = -speedLimit;

	
	car.rotation.y = car.rotation.y % (2*Math.PI);
	aux.setX(Math.cos(-carAngle) * carSpeed);
	aux.setZ(Math.sin(-carAngle) * carSpeed);
	car.position.add(aux);
	
	
	if (carLeft && !carRight) {
		wheelTurn = 1;
	}
	else if (carRight && !carLeft) {
		wheelTurn = -1;
	}
	else {
		wheelTurn = 0;
	}

	if (carSpeed != 0) {
		if ((carLeft && !carRight && carSpeed > 0) || (carRight && !carLeft && carSpeed < 0)) {
			car.rotation.y += carAngularSpeed;
			carAngle += carAngularSpeed;
			carAngle = carAngle % (2*Math.PI);
			//alpha += carAngularSpeed;
			//alpha = fmod(alpha, 360.0f);
			//if (tracking != 1)
				//updateCamera();
		}

		else if (carRight && !carLeft && carSpeed > 0 || (carLeft && !carRight && carSpeed < 0)) {
			car.rotation.y -= carAngularSpeed;
			carAngle -= carAngularSpeed;
			carAngle = carAngle % (2*Math.PI);
			//alpha -= carAngularSpeed;
			//alpha = fmod(alpha, 360.0f);
			//if (tracking != 1)
				//updateCamera();
		}
	}

}

var keyDown = function (event) {
	//movement keys
	if(event.key == 'w'){
		carForward = true;
	}
	else if(event.key == 'a'){
		carLeft = true;
	}
	else if(event.key == 's'){
		carBackward = true;
	}
	else if(event.key == 'd'){
		carRight = true;
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
		carForward = false;
	}
	else if(event.key == 'a'){
		carLeft = false;
	}
	else if(event.key == 's'){
		carBackward = false;
	}
	else if(event.key == 'd'){
		carRight = false;
	}
}

var animate = function(){
	animateCar();
	animateCheerios();
	animateOranges();
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