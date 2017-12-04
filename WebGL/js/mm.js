//global THREE
'use strict';

var scene,
	renderer,
	camera;

var geo, mat, plane;

var directional_light = new THREE.DirectionalLight(0xffffff, 0.5);
var light = new THREE.PointLight( new THREE.Color("rgb(255,70,3)"), 2.5);

geo = new THREE.PlaneGeometry(50,50);
mat = new THREE.MeshPhongMaterial({color: 0x00ffff, specular: 0xffffff, shininess: 5000});

mat = new THREE.MeshBasicMaterial({color: 0x00ffff});

//var manager = new THREE.LoadingManager();
var loader = new THREE.TextureLoader();

var bmap =  loader.load("js/textures/normal.tga");
var map = loader.load("js/textures/stone.tga");

var oldMaterial = new THREE.MeshPhongMaterial({
  //color      :  new THREE.Color("rgb(200,200,200)"),
  color        : 0xffffff,
  //emissive   :  new THREE.Color("rgb(200,200,200)"),
  //specular   :  new THREE.Color("rgb(200,200,200)"),
  shininess  :  20,
  //bumpMap    :  bmap,
  //map        :  map,
  bumpScale  :  0.45,
});

plane = new THREE.Mesh(geo, oldMaterial);

var init = function() {
	scene = new THREE.Scene();
    renderer = new THREE.WebGLRenderer();

    camera = new THREE.PerspectiveCamera(45, 640/480, 0.1, 1000);
    camera.position.x = 50;
    camera.position.y = 50;
    camera.position.z = 50;
    camera.lookAt(new THREE.Vector3(0,0,0));

    renderer.setSize(640, 480);

    scene.add(camera);
    document.body.appendChild( renderer.domElement );

    scene.add(new THREE.AxisHelper(10));

	directional_light.position.set(0, 100, 0);
	//scene.add(directional_light);

    scene.add(light);
    light.position.y = 100;
    light.position.x = 100;

    scene.add(plane);

    plane.rotateOnAxis(new THREE.Vector3(1,0,0), THREE.Math.degToRad(-90));

	renderScene();

};

var render = function (){
		renderer.render(scene, camera);
	};

var renderScene = function() {

	requestAnimationFrame(renderScene);
	render();
};