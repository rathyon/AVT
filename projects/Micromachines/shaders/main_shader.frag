#version 330

uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform int texMode;
uniform float lensAlpha;
uniform float particleAlpha;

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

struct LightProperties {
	bool isEnabled;
	bool isPointLight;
	bool isSpotLight;
	vec4 color;
	vec4 position;
	vec4 halfVector;
	vec3 coneDirection;
	float spotCosCutoff;
	float spotExponent;
	float linearAttenuation;
};

const int numLights = 9; // HAVE TO SET THIS MANUALLY

uniform LightProperties Lights[numLights];

in Data {
	vec3 normal;
	vec3 eye;
	vec3 position;
	vec2 tex_coord;
} DataIn;

in float fogFactor;

void main() {

	vec4 spec = vec4(0.0);
	vec4 diff = vec4(0.0);

	vec4 texel1, texel2;

	vec4 scatteredLight = vec4(0.0);
	vec4 reflectedLight = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);
	vec3 h = vec3(0.0);
	vec3 l = vec3(0.0);

	float strength = 1.0f; //default strength for directional light

	vec4 fogColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	for(int light = 0; light < numLights; light++){
		if(!Lights[light].isEnabled)
			continue;

		if(Lights[light].isPointLight){

			l = normalize(vec3(Lights[light].position) - DataIn.position);
			h = normalize(l + e);

			strength = 1.0f / (length(vec3(Lights[light].position) - DataIn.position)* Lights[light].linearAttenuation);

			// if spotlight...
			if(Lights[light].isSpotLight){
				float spotCos = dot(l, -Lights[light].coneDirection);

				if(spotCos < Lights[light].spotCosCutoff)
					strength = 0.0f;
				else
					strength *= pow(spotCos, Lights[light].spotExponent);
			}

		}

		// directional light
		else {
			strength = 1.0f;
			l = vec3(Lights[light].position);
			h = normalize(vec3(Lights[light].position) + e);
		}

		diff = max(dot(l, n), 0.0) * mat.diffuse * strength;

		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess) * strength;

		scatteredLight += Lights[light].color * diff;
		reflectedLight += Lights[light].color * spec;

	} //end for

	if(texMode == 1){ //table
		texel1 = texture(texmap1, DataIn.tex_coord);
		//texel2 = texture(texmap2, DataIn.tex_coord);

		//vec4 preFogColor = max(strength * texel1 + reflectedLight, 0.01*texel1*texel2);
		vec4 preFogColor = max(scatteredLight * texel1 + reflectedLight, (0.01*texel1));
		preFogColor.a = 1.0f;
		colorOut = mix(fogColor, preFogColor, fogFactor);

	}
	else if(texMode == 2){ //lens flare
		texel2 = texture(texmap2, DataIn.tex_coord);

		if (texel2.a != 0) {
            texel2.a = lensAlpha;
            colorOut = texel2;
		}
        else
           colorOut = texel2;
	}
	else if(texMode == 3){ //billboard
		texel1 = texture(texmap1, DataIn.tex_coord);

		float billX = (DataIn.tex_coord.x - 0.5f) * (DataIn.tex_coord.x - 0.5f);
		float billY = (DataIn.tex_coord.y - 0.5f) * (DataIn.tex_coord.t - 0.5f);

		float distance = sqrt(billX + billY);

		if (distance < 0.45){
			vec4 preFogColor = max(scatteredLight * texel1 + reflectedLight, texel1);
			preFogColor = mix(fogColor, preFogColor, fogFactor);
			colorOut = max(texel1, preFogColor);
			//colorOut = preFogColor; //For full fog effect
		}
		else
			discard;
	}
	else if(texMode == 4) { //pause and game over screens
		texel1 = texture(texmap1, DataIn.tex_coord);
		if (texel1.r < 0.5)
			discard;
		colorOut = vec4(0, 0, 0, 1);
	}
	else if(texMode == 5) { //particles
		texel1 = texture(texmap1, DataIn.tex_coord);
		if (texel1.a < 0.5)
			discard;
		else{
			texel1.a = particleAlpha;
			colorOut = texel1;
		}
	}
	else {

	vec4 rgb = min(scatteredLight + reflectedLight, vec4(1.0));

	vec4 preFogColor = max(rgb, mat.ambient);

	colorOut = mix(fogColor, preFogColor, fogFactor);
	}

}