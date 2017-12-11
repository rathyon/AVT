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

const PARTICLE_NUMBER = 50;

const POINT_LIGHT_INTENSITY = 0.01;
const POINT_LIGHT_COLOR = 0x1226AB;
const particleAcc = [ 0.1, -0.15, 0 ];

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

// ----------[ Directional Light ]----------\\

var directional_light = new THREE.DirectionalLight(0xFFFACD, 2);
directional_light.position.set(0, 100, 70);

// ----------[ Point Lights ]----------\\

var light1 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light1.position.set(-40, 10, -40);
light1.castShadow = true;
light1.shadow.mapSize.width = 256;
light1.shadow.mapSize.height = 256;
light1.shadow.camera.near = 0.1;
light1.shadow.camera.far = 200;

var light2 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light2.position.set(-40, 10, 40);
light2.castShadow = true;
light2.shadow.mapSize.width = 256;
light2.shadow.mapSize.height = 256;
light2.shadow.camera.near = 0.1;
light2.shadow.camera.far = 200;

/*
var light3 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light3.position.set(40, 10, -40);
light3.castShadow = true;
light3.shadow.mapSize.width = 2048;
light3.shadow.mapSize.height = 2048;
light3.shadow.camera.near = 0.1;
light3.shadow.camera.far = 5000;

var light4 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light4.position.set(40, 10, 40);
light4.castShadow = true;
light4.shadow.mapSize.width = 2048;
light4.shadow.mapSize.height = 2048;
light4.shadow.camera.near = 0.1;
light4.shadow.camera.far = 5000;

var light5 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light5.position.set(30, 10, 0);
light5.castShadow = true;
light5.shadow.mapSize.width = 2048;
light5.shadow.mapSize.height = 2048;
light5.shadow.camera.near = 0.1;
light5.shadow.camera.far = 5000;

var light6 = new THREE.PointLight( POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
light6.position.set(-30, 10, 0);
light6.castShadow = true;
light6.shadow.mapSize.width = 2048;
light6.shadow.mapSize.height = 2048;
light6.shadow.camera.near = 0.1;
light6.shadow.camera.far = 5000;
*/

var pointLights = [ light1, light2 ];

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

mirror.position.set(0, 17.5, -49.5);

// ----------[ Geometry ]----------\\

var geo_loader = new THREE.OBJLoader();

var floor_geo = new THREE.PlaneGeometry(100, 100);

/*
var car_geo = new THREE.BoxGeometry(1, 1, 1);

var cheerio_geo = new THREE.TorusGeometry( 1, 0.5, 16, 30 );

var orange_geo = new THREE.SphereGeometry ( 1, 30, 30 );
*/

var particle_geo = new THREE.Geometry();

var checkpoint0and2_geo = new THREE.PlaneGeometry(21, 2);
var checkpoint1and3_geo = new THREE.PlaneGeometry(2, 21);

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
  color        : 0x3333EE,
  specular   :  0xffffff,
  shininess  :  1000,
});

var orange_mat = new THREE.MeshPhongMaterial({
  color        : 0xFFFF66,
  specular   :  0xffffff,
  shininess  :  1000,
});

var car_mat = new THREE.MeshPhongMaterial({
  color : 0xcc0033,
  specular   :  0xffffff,
  shininess  :  1000,
});

var life_mat = new THREE.MeshBasicMaterial({
  color : 0xcc0033,
});

var particle_map = tex_loader.load('js/textures/particle.png');
var particle_mat = new THREE.PointsMaterial( { size:1.5, map: particle_map, transparent: true } );

var billboard_map = tex_loader.load('js/textures/blue_flame.png');

var billboard_mat = new THREE.SpriteMaterial({ map: billboard_map, color: 0xffffff, side : THREE.FrontSide });

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

var car = new THREE.Object3D();

geo_loader.load('js/models/car.obj', function(object){
		object.traverse( function ( child ) {
			if ( child instanceof THREE.Mesh ) {
				child.material = car_mat;
				child.castShadow = true;
            }
        } );
		
		car.add(object);
});

car.scale.set(2,2,2);

var wall = new THREE.Mesh(floor_geo, floor_mat);
wall.position.set(0,50,-50);

var wall2 = new THREE.Mesh(floor_geo, floor_mat);
wall2.position.set(0,50, 50);
var wall3 = new THREE.Mesh(floor_geo, floor_mat);
wall3.position.set(50,50, 0);
var wall4 = new THREE.Mesh(floor_geo, floor_mat);
wall4.position.set(-50,50, 0);

var billboard = new THREE.Sprite(billboard_mat);
billboard.position.set(0, 15, 0);
billboard.scale.set(20, 20, 1);

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

// ----------[ Spotlight ]----------\\

var spotlight = new THREE.SpotLight(0xfeffa1, 1.5, 20, Math.PI / 4);
spotlight.position.set(car.position.x, 0, car.position.z);
spotlight.castShadow = true;

//Set up shadow properties for the spotlight
spotlight.shadow.mapSize.width = 128;  // default
spotlight.shadow.mapSize.height = 128; // default
spotlight.shadow.camera.near = 20;       // default
spotlight.shadow.camera.far = 300;     // default
spotlight.shadow.camera.fov = 45;

// ----------[ Oranges ]----------\\

var orange = [ ];
var orangeSpeed = [ ];
var orangeAcceleration = [ ];
const orangeRotationFactor = 0.2;

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

// ----------[ Particle System ]----------\\

var particleSystem = new THREE.Points(particle_geo, particle_mat);
var particleSpeed = [ ];
var particleLife = [ ];
var particleFade = [ ];
var drawParticles = false;
var deadParticles = 0;



// ----------[ Init ]----------\\

var camStereo = new THREE.PerspectiveCamera(45, window.innerWidth/window.innerHeight, 0.1, 10000);
camStereo.position.set(10, 10, 10);
camStereo.lookAt(new THREE.Vector3(0,0,0));

var camLeft = new THREE.PerspectiveCamera();
var camRight = new THREE.PerspectiveCamera();
camLeft.lookAt(new THREE.Vector3(5,0,0));
camRight.lookAt(new THREE.Vector3(5,0,0));

function createStereo(aspect, fov, focus, realEyeSep){
	// assumes aspect comes divided by 2

	var near = 0.1;

	var eyeRight = new THREE.Matrix4();
	var eyeLeft = new THREE.Matrix4();

	var projectionMatrix = camStereo.projectionMatrix.clone();
	var eyeSep = realEyeSep / 2;
	// eye separation on projection
	var D = eyeSep * near / focus;
	var hdiv2 = near * Math.tan(THREE.Math.degToRad(fov) / 2);

	var left, right;

	// translate xOffset
	eyeLeft.elements[ 12 ] = - eyeSep;
	eyeRight.elements[ 12 ] = eyeSep;

	// left eye
	left = -aspect * hdiv2 + D;
	right = aspect * hdiv2 + D;

	projectionMatrix.elements[ 0 ] = 2 * near / ( right - left );
	projectionMatrix.elements[ 8 ] = ( right + left ) / ( right - left );
	camLeft.projectionMatrix.copy( projectionMatrix );

	// right eye
	left = -aspect * hdiv2 - D;
	right = aspect * hdiv2 - D;

	projectionMatrix.elements[ 0 ] = 2 * near / ( right - left );
	projectionMatrix.elements[ 8 ] = ( right + left ) / ( right - left );

	camRight.projectionMatrix.copy( projectionMatrix );

	camLeft.matrixWorld.copy( camera.matrixWorld ).multiply( eyeLeft );
	camRight.matrixWorld.copy( camera.matrixWorld ).multiply( eyeRight );

	camLeft.position.y = 0.3;
	camRight.position.y = 0.3;

}

function spawnParticles() {
	var v, phi, theta;
	for (var i = 0 ; i < PARTICLE_NUMBER; i++) {
		v = 0.3 * Math.random() + 0.2;
		phi = Math.random() * Math.PI;
		theta = 2 * Math.random() * Math.PI;
		
		var x = car.position.x;
		var y = car.position.y + 1;
		var z = car.position.z;
		particleSystem.geometry.vertices[i] = new THREE.Vector3(x,y,z);
		
		particleSpeed[i] = new THREE.Vector3(v * Math.cos(theta), v * Math.cos(phi), v * Math.sin(theta) * Math.sin(phi));
		
		particleLife[i] = 1;
		particleFade[i] = 0.01;
	}
}

function iterateParticles()
{
	var i;
	var h;

	h = 0.125;
	if (!isPaused) {
		for (i = 0; i < PARTICLE_NUMBER; i++)
		{

			if (particleSystem.geometry.vertices[i].y < -5) {
				particleLife[i] = 0;
			}

			else {
				particleLife[i] -= particleFade[i];
			}
			
			if (particleLife[i] > 0) {
				particleSystem.geometry.vertices[i].x += h*particleSpeed[i].x;
				particleSystem.geometry.vertices[i].y += h*particleSpeed[i].y;
				particleSystem.geometry.vertices[i].z += h*particleSpeed[i].z;

				particleSpeed[i].x += h*particleAcc[0];
				particleSpeed[i].y += h*particleAcc[1];
				particleSpeed[i].z += h*particleAcc[2];
			}
			
			else
				deadParticles++;
				

		}

	}
	
	

}

function createHUD()
{
	hudScene = new THREE.Scene();
	
	for (var i = 0; i < lives; i++) {
		livesObj[i] = new THREE.Object3D();
		(function(i){
			geo_loader.load('js/models/car.obj', function(object){
			
				object.traverse(function(child){
				if(child instanceof THREE.Mesh){
					child.material = life_mat;
				}
				
				livesObj[i] = object;
				livesObj[i].scale.set(5, 5, 1);
				livesObj[i].position.set(92, 36 - (5*i), 0);
				hudScene.add(livesObj[i]);
				
				});
			});
		})(i); 
	}
	
	var hudCanvas = document.createElement('canvas');
	hudCanvas.width = window.innerWidth;
	hudCanvas.height = window.innerHeight;

	var hudBitmap = hudCanvas.getContext('2d');
	hudBitmap.font = "Normal 40px Arial";
	hudBitmap.textAlign = 'center';
	hudBitmap.fillStyle = "rgba(245,245,245,0.75)";
	hudBitmap.fillText('Initializing...', window.innerWidth / 2, window.innerWidth / 2);

	hudCamera = new THREE.OrthographicCamera(-window.innerWidth / 18, window.innerWidth / 18, window.innerHeight / 18, - window.innerHeight / 18, 0.1, 1000);
	hudCamera.position.z = 10;
	hudCamera.lookAt(hudScene.position);
	
	hudScene.add(hudCamera);
}

var init = function() {
	scene = new THREE.Scene();
	scene.fog = new THREE.FogExp2(0xffffff, 0.02);
	
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
	car.add(camLeft);
	car.add(camRight);

	// createStereo(aspect, fov, focus, realEyeSep)
	createStereo((window.innerWidth/window.innerHeight) / 2, 45, 30, 0.064);

	window.addEventListener('resize', resize);
	
	//Lights
	for (var i = 0; i < pointLights.length ; i++) {
		scene.add(pointLights[i]);
	}
	
	scene.add( spotlight );
	scene.add( spotlight.target );
	scene.add( lensFlare );

	scene.add(directional_light);

	//Axis Helper
    scene.add(new THREE.AxesHelper(10));

	//Objects setup
    scene.add(floor);
    scene.add(wall);
    scene.add(wall2);
    scene.add(wall3);
    scene.add(wall4);
    wall2.rotateY(Math.PI);
    wall3.rotateY(-Math.PI/2);
    wall4.rotateY(Math.PI/2);
    //lay down horizontally
    floor.rotateX(-Math.PI/2);
    scene.add(mirror);
	
    scene.add(car);
	createHUD();
	scene.add(checkPoint0);
	scene.add(checkPoint1);
	scene.add(checkPoint2);
	scene.add(checkPoint3);
	scene.add(billboard);
	
	createCheerios();
	createOranges();

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
	var innerStep = 2*Math.PI / NUMBER_INNER_CHEERIOS;
	for (var i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerioSpeed[i] = 0;
		cheerio[i].position.set(0, 0, 0);
		cheerio[i].translateX(20);
		scene.add(cheerio[i]);
	}

	var outerStep = 2*Math.PI / NUMBER_OUTER_CHEERIOS;
	for (var i = NUMBER_INNER_CHEERIOS; i < NUMBER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerioSpeed[i] = 0;
		cheerio[i].position.set(0, 0, 0);
		cheerio[i].translateX(40);
		scene.add(cheerio[i]);
	}
	
}

function createCheerios() {
	var innerStep = 2*Math.PI / NUMBER_INNER_CHEERIOS;
	for (var i = 0; i < NUMBER_INNER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerio[i] = new THREE.Object3D();
		
		(function(i){
			geo_loader.load('js/models/obstacle.obj', function(object){
				
			object.traverse(function(child){
				if(child instanceof THREE.Mesh) {
					child.material = cheerio_mat;
				}
				
				cheerio[i] = object;
				cheerio[i].rotation.y = (i*innerStep);
				cheerio[i].translateX(10);
				//cheerio[i].castShadow = true;
				scene.add(cheerio[i]);
				});
			});
		})(i); 
	}

	var outerStep = 2*Math.PI / NUMBER_OUTER_CHEERIOS;
	for (var i = NUMBER_INNER_CHEERIOS; i < NUMBER_CHEERIOS; i++) {
		scene.remove(cheerio[i]);
		cheerio[i] = new THREE.Object3D();
		
		(function(i){
			geo_loader.load('js/models/obstacle.obj', function(object){
				
			object.traverse(function(child){
				if(child instanceof THREE.Mesh) {
					child.material = cheerio_mat;
				}
				
				cheerio[i] = object;
				cheerio[i].rotation.y = (i*outerStep);
				cheerio[i].translateX(20);
				cheerio[i].castShadow = true;
				scene.add(cheerio[i]);
				});
			});
		})(i); 
	}
	
}

function createOranges() {
		for (var i = 0; i < NUMBER_ORANGES; i++) {
		scene.remove(orange[i]);
		orange[i] = new THREE.Object3D();
		(function(i){
			geo_loader.load('js/models/spiked_ball.obj', function(object){
				
				object.traverse(function(child){
				if(child instanceof THREE.Mesh) {
					child.material = orange_mat;
				}
				
				orange[i] = object;
				orange[i].position.set(-50, 1, -30+(15*i));
				orange[i].castShadow = true;
				scene.add(orange[i]);
				orangeSpeed[i] = getRandomFloat(0.05, 0.15);
				orangeAcceleration[i] = getRandomFloat(0.001, 0.003);
				
				});
			});
		})(i); 

	}
}

function resetOranges() {
	for (var i = 0; i < NUMBER_ORANGES; i++) {
		scene.remove(orange[i]);
		orange[i].position.set(-50, 1, -30+(15*i));
		orangeSpeed[i] = getRandomFloat(0.05, 0.15);
		orangeAcceleration[i] = getRandomFloat(0.001, 0.003);
		scene.add(orange[i]);
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
			drawParticles = true;
			spawnParticles();
			scene.add(particleSystem);

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
			orange[i].rotation.z += -orangeSpeed[i] * orangeRotationFactor;
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

	var cosCarAngle = Math.cos(-carAngle);
	var sinCarAngle = Math.sin(-carAngle);
	
	aux.setX(cosCarAngle * carSpeed);
	aux.setZ(sinCarAngle  * carSpeed);
	car.position.add(aux);
	spotlight.position.set(car.position.x + 1.6*cosCarAngle, 1, car.position.z + 1.6*sinCarAngle);
	spotlight.target.position.set(car.position.x + 3*cosCarAngle, 0, car.position.z + 3*sinCarAngle);
	
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
		}

		else if (carRight && !carLeft && carSpeed > 0 || (carLeft && !carRight && carSpeed < 0)) {
			car.rotation.y -= carAngularSpeed;
			carAngle -= carAngularSpeed;
			carAngle = carAngle % (2*Math.PI);
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
	switch (event.keyCode) {
		case 87:
			carForward = true;
			break;
		case 65:
			carLeft = true;
			break;
		case 83:
			carBackward = true;
			break;
		case 68:
			carRight = true;
			break;
		case 49:
			camera = camOrtho;
			break;
		case 50:
			camera = camTop;
			break;
		case 51:
			camera = camChase;
			break;
	}
};

var keyUp = function (event) {
	
	switch (event.keyCode) {
		case 87:
			carForward = false;
			break;
		case 65:
			carLeft = false;
			break;
		case 83:
			carBackward = false;
			break;
		case 68:
			carRight = false;
			break;
		case 80: //p
			if (!isGameOver) {
				isPaused = !isPaused;
			}
			break;
		case 82: //r
			resetGame();
			break;
		case 72: //h
			if (spotlight.intensity > 0)
				spotlight.intensity = 0;
			else
				spotlight.intensity = 1.5;
			break;
		case 78: //n
			if (directional_light.intensity > 0)
				directional_light.intensity = 0;
			else
				directional_light.intensity = 2;
			break;
		case 67: //c
			for (var i = 0; i < pointLights.length; i++) {
				if (pointLights[i].intensity > 0)
					pointLights[i].intensity = 0;
				else
					pointLights[i].intensity = POINT_LIGHT_INTENSITY;
			}
			break;
		case 77: //m
			mirror.visible = !mirror.visible;
			break;
	}

};

function animate(){
	if (!isPaused && !isGameOver) {
		animateCar();
		updateScore();
		animateCheerios();
		animateOranges();
		if(drawParticles) {
			iterateParticles();
			particleSystem.geometry.verticesNeedUpdate = true;
			if (deadParticles >= PARTICLE_NUMBER) {
				drawParticles = false;
				deadParticles = 0;
				scene.remove(particleSystem);
			}
		}
	}
}

function resize() {
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

	// createStereo(aspect, fov, focus, realEyeSep)
	createStereo((window.innerWidth/window.innerHeight) / 2, 45, 30, 0.064);
}

function render(){
	renderer.clear();

	/** /
	var size = renderer.getSize();

	renderer.setScissorTest( true );

	renderer.setScissor( 0, 0, size.width / 2, size.height );
	renderer.setViewport( 0, 0, size.width / 2, size.height );

	renderer.render(scene, camLeft);

	renderer.setScissor( size.width / 2, 0, size.width / 2, size.height );
	renderer.setViewport( size.width / 2, 0, size.width / 2, size.height );
	renderer.render(scene, camRight);

	renderer.setScissorTest( false );
	/**/

	// comment the previous section and uncomment this to normal camera scheme
	renderer.render(scene, camera);

	renderer.render(hudScene, hudCamera);
};

function renderScene() {
	requestAnimationFrame(renderScene);
    controls.update();
    animate();
	render();
};