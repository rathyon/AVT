//global THREE
'use strict';

var scene,
	hudScene,
	hudCamera,
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
camChase.position.set(-25, 10, 0);
camChase.lookAt(new THREE.Vector3(0,0,0));

var camOrtho = new THREE.OrthographicCamera(window.innerWidth/-18, window.innerWidth/18, window.innerHeight/18, window.innerHeight/-18, 0.1, 10000);
camOrtho.position.set(0,10,0);
camOrtho.lookAt(new THREE.Vector3(0,0,0));

var camTop = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);
camTop.position.set(75,50,0);
camTop.lookAt(new THREE.Vector3(0,0,0));

//active camera
var camera = camChase;

// ----------[ Diretional Light ]----------\\

var directional_light = new THREE.DirectionalLight(0xffffff, 1.5);
directional_light.position.set(0, 100, 70);

// ----------[ Point Lights ]----------\\

var light1 = new THREE.PointLight( 0x00ffff, 0.5);
light1.position.set(200, 50, 0);
light1.castShadow = true;
light1.shadow.mapSize.width = 2048;
light1.shadow.mapSize.height = 2048;
light1.shadow.camera.near = 0.5;
light1.shadow.camera.far = 500;

var light2 = new THREE.PointLight( 0xffcc66, 2);
light2.position.set(-200, 50, 0);
light2.castShadow = true;
light2.shadow.mapSize.width = 2048;
light2.shadow.mapSize.height = 2048;
light2.shadow.camera.near = 0.5;
light2.shadow.camera.far = 500;

//wtf is wrong with the spotlight ?!?!
var spotlight = new THREE.SpotLight(0xffffff);
spotlight.position.set(0,5, 5);
spotlight.castShadow = true;

//Set up shadow properties for the spotlight
spotlight.shadow.mapSize.width = 512;  // default
spotlight.shadow.mapSize.height = 512; // default
spotlight.shadow.camera.near = 0.5;       // default
spotlight.shadow.camera.far = 500;     // default

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

// ----------[ Mirror ]----------\\

var mirror = new THREE.Reflector( 70, 35, {
	clipBias: 0.003,
	textureWidth: window.innerWidth * window.devicePixelRatio,
	textureHeight: window.innerHeight * window.devicePixelRatio,
	color: 0x777777,
	recursion: 1
} );

mirror.position.set(0, 17.5, -50);

// ----------[ Geometry ]----------\\

var geo_loader = new THREE.OBJLoader();

var floor_geo = new THREE.PlaneGeometry(100, 100);

var car_geo = new THREE.BoxGeometry(1, 1, 1);

var cheerio_geo = new THREE.TorusGeometry( 1, 0.5, 16, 30 );

var orange_geo = new THREE.SphereGeometry ( 1, 30, 30 );

var checkpoint0and2_geo = new THREE.PlaneGeometry(21, 2);
var checkpoint1and3_geo = new THREE.PlaneGeometry(2, 21);
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

var life_mat = new THREE.MeshBasicMaterial({
  color : 0x666666,
  map: map
});

var map0and2 = tex_loader.load('js/textures/checkpoint.png');
map0and2.wrapS = THREE.RepeatWrapping;
map0and2.wrapT = THREE.RepeatWrapping;
map0and2.repeat.set(4, 1);

var map1and3 = tex_loader.load('js/textures/checkpoint.png');
map1and3.wrapS = THREE.RepeatWrapping;
map1and3.wrapT = THREE.RepeatWrapping;
map1and3.repeat.set(1, 3);

var checkpoint0and2_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  0xffffff,
  shininess  :  1000,
  map	: 	map0and2
});

var checkpoint1and3_mat = new THREE.MeshPhongMaterial({
  color        : 0x666666,
  specular   :  0xffffff,
  shininess  :  1000,
  map	: 	map1and3
});

var debugMat = new THREE.MeshBasicMaterial({color: 0xff0000});

// ----------[ Meshes ]----------\\

var floor = new THREE.Mesh(floor_geo, floor_mat);
floor.receiveShadow = true;

var car = new THREE.Mesh(car_geo, floor_mat);
car.castShadow = true;

var wall = new THREE.Mesh(floor_geo, floor_mat);
wall.position.set(0,50,-50.5);

var checkPoint0 = new THREE.Mesh(checkpoint0and2_geo, checkpoint0and2_mat);
checkPoint0.rotateX(-Math.PI / 2);
checkPoint0.position.set(30, 0.01, 0);

var checkPoint2 = new THREE.Mesh(checkpoint0and2_geo, checkpoint0and2_mat);
checkPoint2.rotateX(-Math.PI / 2);
checkPoint2.position.set(-30, 0.01, 0);

var checkPoint1 = new THREE.Mesh(checkpoint1and3_geo, checkpoint1and3_mat);
checkPoint1.rotateX(-Math.PI / 2);
checkPoint1.position.set(0, 0.01, -30);

var checkPoint3 = new THREE.Mesh(checkpoint1and3_geo, checkpoint1and3_mat);
checkPoint3.rotateX(-Math.PI / 2);
checkPoint3.position.set(0, 0.01, 30);


// ----------[ Cheerios ]----------\\

var cheerio = [ ];
var cheerioSpeed = [ ];
var cheerioDir = [ ];

for(var i = 0; i < NUMBER_CHEERIOS; i++) {
    cheerioDir[i] = new THREE.Vector3();
}

const cheerioTraction = 0.003;

// ----------[ Car ]----------\\

const carInitPos = new THREE.Vector3(30, 0.5, 0);
car.position.set(carInitPos.x, carInitPos.y, carInitPos.z);

var carSpeed = 0;
const carInitAngle = -Math.PI / 2;
var carAngle = carInitAngle;
car.rotation.y = carInitAngle;
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

// ----------[ Game Logic ]----------\\

var lives = 5;
var livesObj = [ ];
var isGameOver = false;
var isPaused = false;
var score = 0;
var currentCheckPoint = 0;


// ----------[ Init ]----------\\

function createHUD()
{
	hudScene = new THREE.Scene();
	
	for (var i = 0; i < lives; i++) {
		livesObj[i] = new THREE.Mesh( car_geo.clone(), life_mat )
		livesObj[i].scale.set(5, 5, 1);
		livesObj[i].position.set(92-(i*8), 47, 0);
		hudScene.add(livesObj[i]);
	}

	hudCamera = new THREE.OrthographicCamera(-window.innerWidth / 18, window.innerWidth / 18, window.innerHeight / 18, - window.innerHeight / 18, 0.1, 10000);
	hudCamera.position.z = 10;
	hudCamera.lookAt(hudScene.position);
	
	hudScene.add(hudCamera);
}

var init = function() {
	scene = new THREE.Scene();
	
	//Window
    renderer = new THREE.WebGLRenderer({alpha : true});
    renderer.setClearColor(0x000000, 1.0);
    renderer.setSize(window.innerWidth, window.innerHeight);
	renderer.autoClear = false;
	renderer.shadowMap.enabled = true;
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
	spotlight.visible = false;
	scene.add( lensFlare );

	//scene.add(directional_light);

	//Axis Helper
    scene.add(new THREE.AxesHelper(10));

	//Objects setup
    scene.add(floor);
    scene.add(wall);
    //lay down horizontally
    floor.rotateX(THREE.Math.degToRad(-90));
    scene.add(mirror);

    scene.add(car);
	createHUD();
	scene.add(checkPoint0);
	scene.add(checkPoint1);
	scene.add(checkPoint2);
	scene.add(checkPoint3);
	
	resetCheerios();
	resetOranges();

	//Mouse Controls
    controls = new THREE.OrbitControls( camera, renderer.domElement );
    controls.addEventListener('change', render);

    //Key Down/Up
    document.addEventListener('keydown', keyDown);
    document.addEventListener('keyup', keyUp);

	renderScene();

};

function detectCollision (obj1, obj2) {
	var boundingBoxObj1 = new THREE.Box3().setFromObject(obj1);
	var boundingBoxObj2 = new THREE.Box3().setFromObject(obj2);
	
	return boundingBoxObj1.intersectsBox(boundingBoxObj2);
}

function updateScore() {

	switch (currentCheckPoint) {
	case 0:  // if car's last checkpoint was starting line

		if (detectCollision(car, checkPoint1)) {
			currentCheckPoint++;
		}

		break;
	case 1:

		if (detectCollision(car, checkPoint2)) {
			currentCheckPoint++;
		}
		
		break;
	case 2:

		if (detectCollision(car, checkPoint3)) {
			currentCheckPoint++;
		}

		break;
	case 3:

		if (detectCollision(car, checkPoint0)) {
			currentCheckPoint = 0;
			score++;
		}

		break;
	}
}

function resetCheerios() {
	var innerStep = 360 / NUMBER_INNER_CHEERIOS;
	for (var i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerio[i] = new THREE.Mesh(cheerio_geo, cheerio_mat);
		cheerio[i].rotateY(i*innerStep);
		cheerio[i].translateX(20);
		cheerio[i].rotateX(Math.PI / 2);
		cheerio[i].position.y += 0.5;
		cheerio[i].castShadow = true;
		scene.add(cheerio[i]);
	}

	var outerStep = 360 / NUMBER_OUTER_CHEERIOS;
	for (var i = NUMBER_INNER_CHEERIOS; i < NUMBER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerio[i] = new THREE.Mesh(cheerio_geo, cheerio_mat);
		cheerio[i].rotateY(i*outerStep);
		cheerio[i].translateX(40);
		cheerio[i].rotateX(Math.PI / 2);
		cheerio[i].position.y += 0.5;
		cheerio[i].castShadow = true;
		scene.add(cheerio[i]);
	}
	
}

function resetOranges() {
	for (var i = 0; i < NUMBER_ORANGES; i++) {
		scene.remove(orange[i]);
		orange[i] = new THREE.Mesh(orange_geo, orange_mat);
		orange[i].position.set(-50, 1, -30+(15*i));
		scene.add(orange[i]);
		orange[i].castShadow = true;

		orangeSpeed[i] = getRandomFloat(0.15, 0.45);
		orangeAcceleration[i] = getRandomFloat(0.001, 0.004);
	}
}

function resetCar() {
	carSpeed = 0;
	carAngle = carInitAngle;
	car.rotation.y = carInitAngle;

	car.position.set(carInitPos.x, carInitPos.y, carInitPos.z);
}

function carDie() {
	hudScene.remove(livesObj[lives-1]);
	if (lives > 1) {
		lives--;
		currentCheckPoint = 0;
		resetCar();
		resetOranges();
	}
	
	else {
		lives--;
		isGameOver = true;
	}
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

		if (detectCollision(car, orange[i])) {
			carDie();
		}



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


	if (car.position.x < -50 || car.position.x > 50 || car.position.z > 50 || car.position.z < -50)  // lose life if car is out of bounds
		carDie();


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

function resetGame() {
	isGameOver = false;
	isPaused = false;
	lives = 5;
	score = 0;
	currentCheckPoint = 0;
	resetCar();
	resetOranges();
	resetCheerios();
}

var keyDown = function (event) {
	//movement keys
	if (event.keyCode == 87){ //w
		carForward = true;
	}
	
	else if (event.keyCode == 65){ //a
		carLeft = true;
	}
	
	else if (event.keyCode == 83){ //s
		carBackward = true;
	}
	
	else if (event.keyCode == 68){ //d
		carRight = true;
	}
	
	//camera switching
	else if (event.key == '1'){
		camera = camOrtho;
	}
	
	else if (event.key == '2'){
		camera = camTop;
	}
	
	else if (event.key == '3'){
		camera = camChase;
	}
};

var keyUp = function (event) {
	
	if (event.keyCode == 87){ //w
		carForward = false;
	}
	
	else if (event.keyCode == 65){ //a
		carLeft = false;
	}
	
	else if (event.keyCode == 83){ //s
		carBackward = false;
	}
	
	else if (event.keyCode == 68){ //d
		carRight = false;
	}
	
	else if (event.keyCode == 80 && !isGameOver){ //p
		isPaused = !isPaused;
	}
	
	else if (event.keyCode == 82){ //r
		resetGame();
	}

}

var animate = function(){
	if (!isPaused && !isGameOver) {
		animateCar();
		updateScore();
		animateCheerios();
		animateOranges();
	}
};

var resize = function() {
	renderer.setSize(window.innerWidth, window.innerHeight);

	camChase.aspect = window.innerWidth/window.innerHeight;
	camChase.updateProjectionMatrix();

	camOrtho.left = window.innerWidth/-18;
	camOrtho.right = window.innerWidth/18;
	camOrtho.top = window.innerHeight/18;
	camOrtho.bottom = window.innerHeight/-18;
	camOrtho.updateProjectionMatrix();

	camTop.aspect = window.innerWidth/window.innerHeight;
	camTop.updateProjectionMatrix();
};

var render = function (){
		renderer.clear();
		renderer.render(scene, camera);
		renderer.render(hudScene, hudCamera);
};

var renderScene = function() {
	requestAnimationFrame(renderScene);
    controls.update();
    animate();
	render();
};