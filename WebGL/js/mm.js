//global THREE
'use strict';

var scene,
	renderer,
	camera;

var geo, mat, plane;

var directional_light = new THREE.DirectionalLight(0xffffff, 0.5);

geo = new THREE.PlaneGeometry(50,50);
mat = new THREE.MeshPhongMaterial({color: 0x00ffff, specular: 0xffffff, shininess: 5000});

mat = new THREE.MeshBasicMaterial({color: 0x00ffff});

plane = new THREE.Mesh(geo, mat);

var init = function() {
	scene = new THREE.Scene();
    renderer = new THREE.WebGLRenderer();

    camera = new THREE.PerspectiveCamera(45, 640/480, 0.1, 1000);
    camera.position.x = 200;
    camera.position.y = 200;
    camera.position.z = 200;
    camera.lookAt(new THREE.Vector3(0,0,0));

    renderer.setSize(640, 480);

    scene.add(camera);
    document.body.appendChild( renderer.domElement );

    scene.add(new THREE.AxisHelper(10));

	directional_light.position.set(0, -200, 1000);
	scene.add(directional_light);

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