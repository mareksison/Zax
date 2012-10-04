#version 120

attribute vec3 position;

varying vec4 out_color;

//try changing/animating the uniform below in your code.
uniform vec2 offset;
uniform bool isDead;

void main() {
	out_color = vec4(position.z, (vec2(position.x, position.y) + vec2(1.0,1.0))/2.0, 1.0);
	/*if (isDead){
		out_color = vec4(0.5, 0, 0, 1.0);
	}
	else{
		out_color = vec4(0.5, 0.5, 0.5, 1.0);
	}
	out_color = out_color;*/

	//gl_Position = vec4((position.xy + offset) * (position.z * 0.8 + 0.2) ,0.0, 1.0);
	gl_Position = vec4(position.xyz, 1.0);
}
