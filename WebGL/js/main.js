/*global THREE*/
'use strict';

/* ---------[ SCENE VARIABLES ]--------- */

var scene,
    renderer,
	activeCamera,
    cameraOrtho,
	cameraShip,
	cameraStatic,
    aliens,
	bullets,
    ship;

/* ---------[ CAMERA VARIABLES ]--------- */

var WIDTH = 1440,
	HEIGHT = 900,
	ZOOM = 1.5,
    aspectRatio = WIDTH/HEIGHT;
    
/* ---------[ SHIP CAMERA VARIABLES ]--------- */

var cameraShipFOV = 45,
	cameraShipWidth = WIDTH,
    cameraShipHeight = HEIGHT,
    cameraShipNear = 0.1,
    cameraShipFar = 1000;

/* ---------[ STATIC CAMERA VARIABLES ]--------- */	

var cameraStaticFOV = 45,
	cameraStaticWidth = WIDTH,
    cameraStaticHeight = HEIGHT,
    cameraStaticNear = 0.1,
    cameraStaticFar = 1000;

/* ---------[ ORTHO CAMERA VARIABLES ]--------- */

var cameraOrthoWidth = WIDTH,
    cameraOrthoHeight = HEIGHT,
    cameraOrthoNear = 0.1,
    cameraOrthoFar = 1000;
    
/* ----------[ LIGHTING VARIABLES ]----------  */

var daylight = 0.5;
var light_distance = 1000;
var star_brightness = 0.8;
var star_height = 100;

var lighting_enabled = true;

var directional_light = new THREE.DirectionalLight(0xffffff, daylight);

// PointLight( color, intensity, distance, decay )
var p0_light = new THREE.PointLight(0xff0000, star_brightness, light_distance, 6);
var p1_light = new THREE.PointLight(0xffff00, star_brightness, light_distance, 6);
var p2_light = new THREE.PointLight(0x00ffff, star_brightness, light_distance, 6);
var p3_light = new THREE.PointLight(0x0000ff, star_brightness, light_distance, 6);
var p4_light = new THREE.PointLight(0xff00ff, star_brightness, light_distance, 6);
var p5_light = new THREE.PointLight(0x00ff00, star_brightness, light_distance, 6);

var p_lights = [];
p_lights.push(p0_light);
p_lights.push(p1_light);
p_lights.push(p2_light);
p_lights.push(p3_light);
p_lights.push(p4_light);
p_lights.push(p5_light);

/* ----------[ MODELLING VARIABLES ]----------  */

var basic_mat_red = new THREE.MeshBasicMaterial({ color: 0xde1717, wireframe: false });
var basic_mat_green = new THREE.MeshBasicMaterial({ color: 0x17de2e, wireframe: false });
var basic_mat_blue = new THREE.MeshBasicMaterial({ color: 0x2194ce, wireframe: false });
var basic_mat_white = new THREE.MeshBasicMaterial({ color: 0xffffff, wireframe: false });
var basic_mat_black = new THREE.MeshBasicMaterial({ color: 0x000000, wireframe: false });
var basic_mat_gray = new THREE.MeshBasicMaterial({ color: 0x888888, wireframe: false });

var diffuse_mat_red = new THREE.MeshLambertMaterial({color: 0xde1717});
var diffuse_mat_green = new THREE.MeshLambertMaterial({color: 0x17de2e});
var diffuse_mat_blue = new THREE.MeshLambertMaterial({color: 0x2194ce});

var phong_red = new THREE.MeshPhongMaterial({color: 0xde1717, specular: 0x3c3c3c, shininess: 16});
var phong_green = new THREE.MeshPhongMaterial({color: 0x17de2e, specular: 0x3c3c3c, shininess: 16});
var phong_blue = new THREE.MeshPhongMaterial({color: 0x2194ce, specular: 0x3c3c3c, shininess: 16});

var diffuse_blue = new THREE.MeshLambertMaterial({color: 0x2194ce});

/* ---------[ BACKGROUND VARIABLES ]--------- */

var background_geo = new THREE.PlaneGeometry(3840, 2400);

var loader = new THREE.TextureLoader();

var background_texture = loader.load('textures/space.jpg');
background_texture.wrapS = THREE.RepeatWrapping;
background_texture.wrapT = THREE.RepeatWrapping;

var background_mat = new THREE.MeshPhongMaterial({color: 0xffffff, side: THREE.DoubleSide, map: background_texture});
var background = new THREE.Mesh(background_geo, background_mat);

/* ---------[ CLOCK VARIABLES ]--------- */

var clock,
    time,
    delta;

/* ---------[ SHIP VARIABLES ]--------- */
	
var speed = 0;
var acceleration = 0;
var stopping = false;

//Limits for ship movement
var ship_left_limit = -250,
    ship_right_limit = 250;

/* -----------[ BULLET CLASS ]---------- */

var bullet_speed = 300;
    
function Bullet(x, y, z){
    this.mesh = createBullet(x, y, z);
    
    var center = new THREE.Vector3(x, y, z);
    
    //cube is size 5
    this.boundary = new THREE.Sphere(center, Math.sqrt(50)/2);
   
	this.dead = false;
    
    this.speed_x = 0;
    this.speed_y = bullet_speed;
};

//returns 3D Object
var createBullet = function (x, y, z) {
	//cria um objecto de threejs
	var group = new THREE.Object3D();

	addCube(group, 0, 0, 1, 1);
	
	group.traverse( function(node) {
	    if(node instanceof THREE.Mesh){
	        node.material = basic_mat_red;
	    }
	});
	
	// define a posicao inicial do tiro
	group.position.x = x;
	group.position.y = y;
	group.position.z = z;
	
	scene.add(group);
	
	return group;
};

Bullet.prototype.move = function(x, y){
    this.mesh.position.x = x;
    this.mesh.position.y = y;
    var new_center = new THREE.Vector3(x, y, 0);
    this.boundary.center = new_center;
};

var killBullet = function(bullet){
	if (bullet.dead)
	{
		return;
	}
	bullet.dead = true;
	scene.remove(bullet.mesh);	
}

var animateBullet = function(bullet){
	if (bullet.dead)
	{
		return;
	}
    else if (bullet.mesh.position.y > upper_limit){
        killBullet(bullet);
        return;
    }
  var test_x = bullet.mesh.position.x + bullet.speed_x*delta;
  var test_y = bullet.mesh.position.y + bullet.speed_y*delta;
  
  for(var i=0; i<aliens.length; i++){
	  if (aliens[i].dead)
	  {
		  continue;
	  }
	  
      var test_collision_center = new THREE.Vector3(test_x, test_y, 0);
      var target_center = aliens[i].boundary.center;
      var distanceSquared = test_collision_center.distanceToSquared(target_center);
      var radiusSum = bullet.boundary.radius + aliens[i].boundary.radius;
      
    if(distanceSquared <= radiusSum*radiusSum){
        
		killBullet(bullet);		
		killAlien(aliens[i]);
        
		/*aliens[i].speed_x = -aliens[i].speed_x;
        aliens[i].speed_y = -aliens[i].speed_y;*/
        
        return;
    }
  }
 
	bullet.move(test_x, test_y);
    
};
    
/* ---------[ ALIEN CLASS ]--------- */

var alien_speed = 20;

var two_pi = 2*Math.PI;

var left_limit = -250,
    right_limit = 250,
    bottom_limit = -50,
    upper_limit = 200;

   
function Alien(x, y, z){
    this.mesh = createAlien(x, y, z);
    
    var center = new THREE.Vector3(x, y, z);
    
    //cube is size 5
    this.boundary = new THREE.Sphere(center, Math.sqrt(10.25*10)); //changed value for quality, accurate value is sqrt(10.25)*5
    
    var theta = Math.random() * two_pi;
	
	this.dead = false;
    
    this.speed_x = alien_speed*(Math.cos(theta));
    this.speed_y = alien_speed*(Math.sin(theta));
};

var killAlien = function(alien){
	if (alien.dead)
	{
		return;
	}
	alien.dead = true;
	scene.remove(alien.mesh);	
}

var animateAlien = function(alien){
	  if (alien.dead)
	  {
		  return;
	  }
    
  var test_x = alien.mesh.position.x + alien.speed_x*delta;
  var test_y = alien.mesh.position.y + alien.speed_y*delta;
  
  for(var i=0; i<aliens.length; i++){
      
      if(alien.mesh.position.x == aliens[i].mesh.position.x && alien.mesh.position.y == aliens[i].mesh.position.y){
          continue;
      }
	  
	  if (aliens[i].dead)
	  {
		  continue;
	  }
      
      var test_collision_center = new THREE.Vector3(test_x, test_y, 0);
      var target_center = aliens[i].boundary.center;
      var distanceSquared = test_collision_center.distanceToSquared(target_center);
      var radiusSum = alien.boundary.radius + aliens[i].boundary.radius;
      
    if(distanceSquared <= radiusSum*radiusSum){
        
        alien.speed_x = -alien.speed_x;
        alien.speed_y = -alien.speed_y;
        aliens[i].speed_x = -aliens[i].speed_x;
        aliens[i].speed_y = -aliens[i].speed_y;
        
        test_x = alien.mesh.position.x + alien.speed_x*delta;
        test_y = alien.mesh.position.y + alien.speed_y*delta;
        
        alien.move(test_x, test_y);
        return;
    }
  }
  
  if(test_x > right_limit){
      alien.move(right_limit, test_y);
      alien.speed_x = -alien.speed_x;
  } 
  else if(test_x < left_limit){
      alien.move(left_limit, test_y);
      alien.speed_x = -alien.speed_x;
  }
  else if(test_y < bottom_limit){
      alien.move(test_x, bottom_limit);
      alien.speed_y = -alien.speed_y;
  }
  else if(test_y > upper_limit){
      alien.move(test_x, upper_limit);
      alien.speed_y = -alien.speed_y;
  }
  else {
      alien.move(test_x, test_y);
  }
    
};

Alien.prototype.move = function(x, y){
    this.mesh.position.x = x;
    this.mesh.position.y = y;
    var new_center = new THREE.Vector3(x, y, 0);
    this.boundary.center = new_center;
};

/*---------------------------------------*/

var init = function() {
    scene = new THREE.Scene();
    renderer = new THREE.WebGLRenderer();
	
    cameraOrtho = new THREE.OrthographicCamera( cameraOrthoWidth/-2, cameraOrthoWidth/2, cameraOrthoHeight/2, cameraOrthoHeight/ -2, cameraOrthoNear, cameraOrthoFar);
    cameraOrtho.position.z = 100;
    cameraOrtho.zoom = ZOOM;
	
	cameraShip = new THREE.PerspectiveCamera (cameraShipFOV, cameraShipWidth/cameraShipHeight, cameraShipNear, cameraShipFar);
	cameraShip.position.x = 0;
	cameraShip.position.y = -25;
    cameraShip.position.z = 25;
	cameraShip.lookAt(new THREE.Vector3( 0, 20, 0 ));
	
	cameraStatic = new THREE.PerspectiveCamera (cameraStaticFOV, cameraStaticWidth/cameraStaticHeight, cameraStaticNear, cameraStaticFar);
	cameraStatic.position.x = 0;
	cameraStatic.position.y = -400;
    cameraStatic.position.z = 400;
	cameraStatic.lookAt(new THREE.Vector3( 0, 0, 0 ));
	
	activeCamera = cameraOrtho;
	
	
	directional_light.position.set(0, -200, 1000);
	scene.add(directional_light);
	
	p0_light.position.set(0, 0, star_height);
	scene.add(p0_light);
	p1_light.position.set(150, 150, star_height);
	scene.add(p1_light);
	p2_light.position.set(-150, -150, star_height);
	scene.add(p2_light);
	p3_light.position.set(-100, 100, star_height);
	scene.add(p3_light);
	p4_light.position.set(125, -125, star_height);
	scene.add(p4_light);
	p5_light.position.set(0, -200, star_height);
	scene.add(p5_light);
	
	background.position.z = -20;
	scene.add(background);
	
	//LIGHT HELPERS
	/*
	scene.add(new THREE.PointLightHelper(p0_light, 5));
	scene.add(new THREE.PointLightHelper(p1_light, 5));
	scene.add(new THREE.PointLightHelper(p2_light, 5));
	scene.add(new THREE.PointLightHelper(p3_light, 5));
	scene.add(new THREE.PointLightHelper(p4_light, 5));
	scene.add(new THREE.PointLightHelper(p5_light, 5));
	*/
    renderer.setSize(WIDTH, HEIGHT);
    scene.add(cameraOrtho);
	scene.add(cameraStatic);
    document.body.appendChild( renderer.domElement );
    
    ship = createPolyMesh(0, -200, 0);
    ship.scale.x = 5;
    ship.scale.y = 5;
    ship.scale.z = 5;
    scene.add(ship);
	ship.add(cameraShip);
    
    aliens = [];
    for (var rows=1; rows<=2; rows++){ 
		for (var columns=-4; columns<5; columns++){	
			var alien = new Alien(columns * 30, rows * 25, 0);		
			aliens.push(alien);
		}
	}
	
	bullets = [];
	
	//scene.add(new THREE.AxisHelper(10));
	
	clock = new THREE.Clock();
	
	resize();
	update();
    
};

var render = function (){
		renderer.render(scene, activeCamera);
	};

var resize = function(){
    
    var widthRatio = window.innerWidth/WIDTH;
    var heightRatio = window.innerHeight/HEIGHT;
    var minRatio = Math.min(widthRatio, heightRatio);
	var newZoom;
	
    cameraOrthoWidth = WIDTH*minRatio;
    cameraOrthoHeight = HEIGHT*minRatio;
    newZoom = ZOOM*minRatio;
	
    cameraOrtho.left = cameraOrthoWidth/-2;
    cameraOrtho.right = cameraOrthoWidth/2;
    cameraOrtho.top = cameraOrthoHeight/2;
    cameraOrtho.bottom = cameraOrthoHeight/-2;
    cameraOrtho.zoom = newZoom;
	
    renderer.setSize(cameraOrthoWidth, cameraOrthoHeight);
    cameraOrtho.updateProjectionMatrix();
    
};

var checkKeyDown = function (e) {
	
    e = e || window.event;

	if (e.keyCode == '66' || e.keyCode == '98') // 'b' or 'B'
	{
		var bullet = new Bullet(ship.position.x, ship.position.y + 10, 0);		
		bullets.push(bullet);		
	}
	else if (e.keyCode == '37') { // left arrow
		acceleration = -800;
    }
    else if (e.keyCode == '39') { // right arrow
		acceleration = 800;
    }
    else if(e.keyCode == '65' || e.keyCode == '97'){ // 'a' or 'A'
        scene.traverse( function (node){
            if(node instanceof THREE.Mesh){
                node.material.wireframe = !node.material.wireframe;
            }
        });
        
        for(var i=0; i<aliens.length; i++){
            aliens[i].mesh.material.wireframe = !aliens[i].mesh.material.wireframe;
        }
        
    }
	else if (e.keyCode == '49') { // 1
		activeCamera = cameraOrtho;
    }
	else if (e.keyCode == '50') { // 2
		activeCamera = cameraShip;
    }
	else if (e.keyCode == '51') { // 3
		activeCamera = cameraStatic;
    }
    else if (e.keyCode == '78') { // n
        if(lighting_enabled){
            if(directional_light.visible) {
                directional_light.visible = false;
            }
            else if(!directional_light.visible){
                directional_light.visible = true;
            }
        }
    }
    else if (e.keyCode == '71') { // g
        if(lighting_enabled){
            scene.traverse( function (node){
                if(node instanceof THREE.Mesh){
                    if(node.material.id == diffuse_mat_red.id){
                        node.material = phong_red;
                    }
                    else if(node.material.id == diffuse_mat_green.id){
                        node.material = phong_green;
                    }
                    else if(node.material.id == diffuse_mat_blue.id){
                        node.material = phong_blue;
                    }
                    else if(node.material.id == phong_red.id){
                        node.material = diffuse_mat_red;
                    }
                    else if(node.material.id == phong_green.id){
                        node.material = diffuse_mat_green;
                    }
                    else if(node.material.id == phong_blue.id){
                        node.material = diffuse_mat_blue;
                    }
                }
            });
        }
            
    }
    else if (e.keyCode == '76') { // L
		if(lighting_enabled){
		    lighting_enabled = false;
		    directional_light.visible = false;
		    for(var i = 0; i < p_lights.length; i++){
		        p_lights[i].visible = false;
		    }
		    scene.traverse(function (node){
		        if(node instanceof THREE.Mesh){
		            var mat_id = node.material.id;
		            if(mat_id == diffuse_mat_red.id || mat_id == phong_red.id){
		                node.material = basic_mat_red;
		            }
		            else if(mat_id == diffuse_mat_green.id || mat_id == phong_green.id){
		                node.material = basic_mat_green;
		            }
		            else if(mat_id == diffuse_mat_blue.id || mat_id == phong_blue.id){
		                node.material = basic_mat_blue;
		            }
		        }
            });
		}
		else if(!lighting_enabled){
		    lighting_enabled = true;
		    directional_light.visible = true;
		    for(var i = 0; i < p_lights.length; i++){
		        p_lights[i].visible = true;
		    }
		    scene.traverse(function (node){
		        if(node instanceof THREE.Mesh){
		            var mat_id = node.material.id;
		            if(mat_id == basic_mat_red.id){
		                node.material = phong_red;
		            }
		            else if(mat_id == basic_mat_green.id){
		                node.material = phong_green;
		            }
		            else if(mat_id == basic_mat_blue.id){
		                node.material = phong_blue;
		            }
		        }
            });
		}
    }
	else if (e.keyCode == '67' || e.keyCode == '99'){ // 'c' or 'C'
	    if(lighting_enabled){
		var i = 0;
    		if(p_lights[0].visible){
    			while(i < p_lights.length){
    				p_lights[i].visible = false;
    				i++;
    			}
    		}
    		else if(!p_lights[0].visible){
    			while(i < p_lights.length){
    				p_lights[i].visible = true;
    				i++;
    			}
    		}
	    }
	}
};

var checkKeyUp = function (e) {

    e = e || window.event;

    if (e.keyCode == '37') { // left arrow
		acceleration = 1000;
		stopping = true;
	}
	else if(e.keyCode == '39') { // right arrow
		acceleration = -1000;
		stopping = true;
    }
};

document.addEventListener('keydown', checkKeyDown);
document.addEventListener('keyup', checkKeyUp);
window.addEventListener('resize', resize);

// returns 3D Object
var createOldShip = function(x, y, z) {
	var ship = new THREE.Object3D();
	
	var hull_geo = new THREE.CylinderGeometry(1.5, 1.5, 4, 16);
    var back_piece_geo = new THREE.CylinderGeometry(0.7, 0.7, 2.5, 32);
    var front_piece_geo = new THREE.SphereGeometry(3.5, 16, 16);
    var cockpit_geo = new THREE.SphereGeometry(1.7, 16, 16);
    var thruster_geo = new THREE.CylinderGeometry(1, 1, 3, 16);
    
    var hull = new THREE.Mesh(hull_geo, diffuse_mat_red);
    var back_piece = new THREE.Mesh(back_piece_geo, diffuse_mat_blue);
    var front_piece = new THREE.Mesh(front_piece_geo, diffuse_mat_blue);
    var cockpit = new THREE.Mesh(cockpit_geo, diffuse_mat_blue);
    var thruster_right = new THREE.Mesh(thruster_geo, diffuse_mat_red);
    var thruster_left = new THREE.Mesh(thruster_geo, diffuse_mat_red);
    
    hull.scale.y = 0.8;
    back_piece.scale.x = 5;
    front_piece.scale.x = 0.9;
    front_piece.scale.y = 1.1;
    front_piece.scale.z = 0.2;
    cockpit.scale.x = 0.75;
    cockpit.scale.y = 1.4;
    cockpit.scale.z = 0.6;
    
    hull.position.set(0, 3.5, 1.3);
    back_piece.position.set(0, 2.25, 1);
    front_piece.position.set(0, 5.5, 1);
    cockpit.position.set(0, 6, 1.3);
    thruster_right.position.set(3, 1.5, 0.8);
    thruster_left.position.set(-3, 1.5, 0.8);
	
	ship.add(hull);
    ship.add(back_piece);
    ship.add(front_piece);
    ship.add(cockpit);
    ship.add(thruster_right);
    ship.add(thruster_left);
    
    ship.position.x = x;
    ship.position.y = y;
    ship.position.z = z;

	return ship;
};

var createPolyMesh = function(x, y, z){
    
    var ship = new THREE.Object3D();
    
    var geometry = new THREE.Geometry();
    
    geometry.vertices.push(
        new THREE.Vector3(-2,6,1),
        new THREE.Vector3(-4,4,1),
        new THREE.Vector3(-5,1,1),
        new THREE.Vector3(-4,-2,1),
        new THREE.Vector3(-1,-4,1),
        new THREE.Vector3(0,-6,1),
        new THREE.Vector3(1,-4,1),
        new THREE.Vector3(4,-2,1),
        new THREE.Vector3(5,1,1),
        new THREE.Vector3(4,4,1),
        new THREE.Vector3(2,6,1),
        new THREE.Vector3(-1,2,3),
        new THREE.Vector3(0,1.25,4),
        new THREE.Vector3(1,2,3),
        
        // underside
        new THREE.Vector3(-2,6,-1),
        new THREE.Vector3(-4,4,-1),
        new THREE.Vector3(-5,1,-1),
        new THREE.Vector3(-4,-2,-1),
        new THREE.Vector3(-1,-4,-1),
        new THREE.Vector3(0,-6,-1),
        new THREE.Vector3(1,-4,-1),
        new THREE.Vector3(4,-2,-1),
        new THREE.Vector3(5,1,-1),
        new THREE.Vector3(4,4,-1),
        new THREE.Vector3(2,6,-1),
        new THREE.Vector3(-1,2,-3),
        new THREE.Vector3(0,1.25,-4),
        new THREE.Vector3(1,2,-3)
    );
        
    geometry.faces.push(
        new THREE.Face3(0,1,11),
        new THREE.Face3(1,2,11),
        new THREE.Face3(2,3,11),
        new THREE.Face3(3,4,11),
        new THREE.Face3(11,4,12),
        new THREE.Face3(12,4,6),
        new THREE.Face3(4,5,6),
        new THREE.Face3(12,6,13),
        new THREE.Face3(6,7,13),
        new THREE.Face3(7,8,13),
        new THREE.Face3(8,9,13),
        new THREE.Face3(9,10,13),
        
        //underside
        new THREE.Face3(25,15,14),
        new THREE.Face3(25,16,14),
        new THREE.Face3(25,17,16),
        new THREE.Face3(25,18,17),
        new THREE.Face3(26,18,25),
        new THREE.Face3(20,18,26),
        new THREE.Face3(20,19,18),
        new THREE.Face3(27,20,26),
        new THREE.Face3(27,21,20),
        new THREE.Face3(27,22,21),
        new THREE.Face3(27,23,22),
        new THREE.Face3(27,24,23),
        
        //connection
        new THREE.Face3(14,1,0),
        new THREE.Face3(14,15,1),
        new THREE.Face3(15,2,1),
        new THREE.Face3(15,16,2),
        new THREE.Face3(16,3,2),
        new THREE.Face3(16,17,3),
        new THREE.Face3(17,4,3),
        new THREE.Face3(17,18,4),
        new THREE.Face3(18,5,4),
        new THREE.Face3(18,19,5),
        new THREE.Face3(5,19,20),
        new THREE.Face3(20,6,5),
        new THREE.Face3(8,22,23),
        new THREE.Face3(23,9,8),
        new THREE.Face3(6,20,21),
        new THREE.Face3(21,7,6),
        new THREE.Face3(7,21,22),
        new THREE.Face3(22,8,7),
        new THREE.Face3(9,23,24),
        new THREE.Face3(24,10,9),
        new THREE.Face3(10,24,13),
        new THREE.Face3(24,27,13),
        new THREE.Face3(0,11,25),
        new THREE.Face3(25,14,0),
        new THREE.Face3(13,11,12),
        new THREE.Face3(27,26,25),
        new THREE.Face3(13,27,11),
        new THREE.Face3(27,25,11)
        
    );
    
    geometry.mergeVertices();
    geometry.computeFaceNormals();
    geometry.computeVertexNormals();
    
    var mesh = new THREE.Mesh(geometry, phong_blue);
        
    ship.add(mesh);
        
    ship.position.x = x;
    ship.position.y = y;
    ship.position.z = z;
        
    return ship;
    
};

//returns 3D Object
var createAlien = function (x, y, z) {
	//cria um objecto de threejs
	var group = new THREE.Object3D();

	// adiciona varios cubos para formar um alien
	addCube(group, 0, 0, 4, 1);
	
	addCube(group, -2, 1, 1, 1);
	addCube(group, 0, 1, 1, 1);
	addCube(group, 2, 1, 1, 1);
	addCube(group, 0, 2, 4, 1);

	addCube(group, -2, -1, 1, 1);
	addCube(group, 2, -1, 1, 1);

	// define a posicao inicial do alien
	
	group.position.x = x;
	group.position.y = y;
	group.position.z = z;
	
	scene.add(group);
	
	return group;
};

//adds a cube to obj
var addCube = function (obj, x, y, width, height) {
	var size = 5; // define o tamanho dos cubos
	var geometry = new THREE.BoxGeometry(size * width, size * height, size, 1, 1, 1);
	var mesh = new THREE.Mesh(geometry, diffuse_mat_green);
	mesh.position.set(x * size, y * size, 0);
	obj.add(mesh);
};

var animateShip = function(){
	
	if(acceleration*speed > 0 && stopping){
        speed = 0;
        acceleration = 0;
        stopping = false;
    } else {
        speed += acceleration*delta;
    }
    
    var testPos = ship.position.x + speed*delta;
    
    if(testPos > ship_right_limit){
        ship.position.x = ship_right_limit-1;
        acceleration = 0;
        speed = 0;
    } else if(testPos < ship_left_limit){
        ship.position.x = ship_left_limit+1;
        acceleration = 0;
        speed = 0;
    } else {
        ship.position.x = testPos;
    }
	
};

//updates the scene
var update = function () {
	
	delta = clock.getDelta();
	
	animateShip();
	
	for(var i=0; i<aliens.length; i++){
	    animateAlien(aliens[i]);
        if(aliens[i].dead){
	        aliens.splice(i, 1);
            i=-1;
	    }
	}
	
	for(var i=0; i<bullets.length; i++){
	    animateBullet(bullets[i]);
        if(bullets[i].dead){
	        bullets.splice(i, 1);
            i=-1;
	    }
	}

	requestAnimationFrame(update);
	render();
	
};