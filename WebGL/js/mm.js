//global THREE
'use strict';

var scene,
	renderer,
	camera,
    controls;

var geo, mat, plane;

//Directional Light
var directional_light = new THREE.DirectionalLight(0xffffff, 0.5);
directional_light.position.set(0, 100, 0);

//Point Light
var light = new THREE.PointLight( new THREE.Color("rgb(255,255,255)"), 2.5);
light.position.y = 100;
light.position.x = 100;


geo = new THREE.PlaneGeometry(50,50);
mat = new THREE.MeshPhongMaterial({color: 0x00ffff, specular: 0xffffff, shininess: 5000});

mat = new THREE.MeshBasicMaterial({color: 0x00ffff});

var loader = new THREE.TextureLoader();

var bmap = loader.load('js/textures/normal.png');
var map = loader.load('js/textures/normal.png');

var oldMaterial = new THREE.MeshPhongMaterial({
  //color      :  new THREE.Color("rgb(200,200,200)"),
  color        : 0x666666,
  //emissive   :  new THREE.Color("rgb(200,200,200)"),
  //specular   :  new THREE.Color("rgb(200,200,200)"),
  shininess  :  20,
  bumpMap    :  bmap,
  //map        :  map,
  //bumpScale  :  0.45,
});

plane = new THREE.Mesh(geo, oldMaterial);

var init = function() {
	scene = new THREE.Scene();
	
	//Window
    renderer = new THREE.WebGLRenderer();
    renderer.setSize(640, 480);
	document.body.appendChild( renderer.domElement );
	
	//Camera
    camera = new THREE.PerspectiveCamera(45, 640/480, 0.1, 1000);
    camera.position.set(50, 50, 50);
    camera.lookAt(new THREE.Vector3(0,0,0));
	scene.add(camera);
	
	//Point light
	scene.add(light);

	//Axis Helper
    scene.add(new THREE.AxisHelper(10));
	
	//scene.add(directional_light);

	//Ground
    scene.add(plane);
    plane.rotateOnAxis(new THREE.Vector3(1,0,0), THREE.Math.degToRad(-90));

	//Mouse Controls
    controls = new THREE.OrbitControls( camera, renderer.domElement );
    controls.addEventListener( 'change', render );

	renderScene();

};

var render = function (){
		renderer.render(scene, camera);
	};

var renderScene = function() {

	requestAnimationFrame(renderScene);
    controls.update();
	render();
};