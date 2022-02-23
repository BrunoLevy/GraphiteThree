//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

const int MAX_RAY_STEPS = 20;

const float pi = 3.14159265359;

float maxcomp(vec2 v) { return max(v.x, v.y); }
float maxcomp(vec3 v) { return max(v.x, max(v.y, v.z)); }

float mincomp(vec3 v) { return min(v.x, min(v.y, v.z)); }

float sum(vec3 v) { return v.x + v.y + v.z; }

vec2 rotate2d(vec2 v, float a) { return vec2(v.x * cos(a) - v.y * sin(a), v.y * cos(a) + v.x * sin(a)); }

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return x*x*x*(x*(x*6.0 - 15.0) + 10.0);
}

float pulse1(float x) {
	x = x / (1.0 + 2.5 * step(0.0, x));
	x = clamp(abs(x), 0.0, 1.0);
	return 1.0 - x*x*x*(x*(x*6.0 - 15.0) + 10.0);	
}

float pulse2(float x) {
	x = x / (1.0 + 1.5 * step(0.0, x));
	return 1.0 - smoothstep(0.0, 1.0, abs(x));	
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float sdBox2p(vec3 p, vec3 b1, vec3 b2) {
	vec3 pos = (b1 + b2) / 2.0;
	vec3 b = (b2 - b1) / 2.0;
	return sdBox(p - pos, b);
}

float tileCubeShell(vec3 p, float t) {
	return max(p.y, -min(p.x, p.z));
}

float marchingCube(vec3 p, float t) {
	p.z -= floor(t);
	p.y += 0.5;
	p.z -= 0.5;
	p.yz = rotate2d(p.yz, -mod(t, 1.0) * pi / 2.0);
	p.y -= 0.5;
	p.z += 0.5;
	return sdBox(p, vec3(0.40)) - 0.05;
}

float tumblingCube(vec3 p, float t) {
	t = clamp(t * 0.333, 0.0, 1.0);
	vec3 pos;
	pos.x = mix(0.5, 0.65, t);
	pos.y = -14.0 * t * t + 14.0 * t + mix(-0.5, -0.65, t);
	pos.z = mix(-0.5, 0.65, t);
	float size = mix(0.4, 0.65, pow(t, 4.0));
	float cornerRounding = mix(0.05, 0.1, pow(t, 4.0));
	float rot = min(mix(0.0, 2.0 * pi, 2.0 * smoothstep(0.0, 1.0, t)), 2.0 * pi);
	return sdBox(vec3(p.x - pos.z, rotate2d(p.yz - pos.yz, rot)), vec3(size)) - cornerRounding;	
}

float marchingCubePath(vec3 p, float t) {
	if (t < 0.0) 
		return sdBox(p - vec3(2.5, max(-0.5 - 3.0 * (t + 1.0), 0.5), 0.5), vec3(0.40)) - 0.05;
	else if (t < 2.0) 
		return marchingCube(p - vec3(2.5, 0.5, 0.5), t);
	else if (t < 5.0) 
		return marchingCube(p.zyx * vec3(1.0, 1.0, -1.0) - vec3(2.5, 0.5, -2.5), t - 2.0);
	else if (t < 8.0) 
		return marchingCube(p.zxy * vec3(1.0, -1.0, -1.0) - vec3(2.5, 0.5, -0.5), t - 5.0);
	else if (t < 11.0) 
		return marchingCube(p.yxz * vec3(1.0, -1.0, -1.0) - vec3(-2.5, 0.5, -2.5), t - 8.0);
	else if (t < 14.0)
		return marchingCube(p.yzx * vec3(1.0, -1.0, 1.0) - vec3(-2.5, 0.5, -0.5), t - 11.0);
	else if (t < 16.0)
		return marchingCube(p.xzy * vec3(1.0, -1.0, 1.0) - vec3(2.5, 0.5, -2.5), t - 14.0);
	else if (t < 18.0)
		return marchingCube(p.yzx * vec3(1.0, -1.0, -1.0) - vec3(-0.5, 0.5, -2.5), t - 16.0);
	else
		return tumblingCube(p, t - 18.0);
}

float finalLogoScene(vec3 p, float t) {
	float thickness = 0.1;
	p.y = -p.y;
	float shell = max(sdBox2p(p, vec3(0.0), vec3(3.0)), -sdBox2p(p, vec3(thickness), vec3(8.0)));
	shell = max(shell, -sdBox2p(p, vec3(-8.0), vec3(2.0)));
	shell = max(shell, -sdBox2p(p, vec3(1.0, -1.0, thickness), vec3(4.0, 5.0, 1.0)));
	shell = min(shell, sdBox2p(p, vec3(0.0), vec3(3.0, 1.0, thickness)));
	shell = min(shell, sdBox2p(p, vec3(0.0), vec3(1.4)));
	return shell;
}

float scene(vec3 p, float t) {
	float bigCube = tileCubeShell(p, t);
	float littleCube = marchingCubePath(p, t - 10.0);
	return min(bigCube, littleCube);
}

float getMaterial(vec3 p, float t) {
	float bigCube = tileCubeShell(p, t);
	float littleCube = marchingCubePath(p, t - 10.0);
	if (bigCube < littleCube) return 0.0;
	else return 1.0;
}

float sideY(vec3 p, float t) {
	vec3 p1 = vec3(2.5, p.y, min(0.5 + t, p.z));
	vec3 p2 = vec3(max(3.5 - (t - 1.0), p.x), p.y, 2.5);
	return max(1.0 - min(length(p - p1), length(p - p2)), 0.0) * step(1.0, p.z);
}

float sideX(vec3 p, float t) {
	vec3 p1 = vec3(p.x, min(-1.5 + t, p.y), 2.5);
	vec3 p2 = vec3(p.x, 2.5, max(6.5 - t, p.z));
	return max(1.0 - min(length(p - p1), length(p - p2)), 0.0);
}

float sideZ(vec3 p, float t) {
	vec3 p1 = vec3(min(-2.5 + t, p.x), 2.5, p.z);
	vec3 p2 = vec3(2.5, max(7.5 - t, p.y), p.z);
	vec3 p3 = vec3(max(max(9.5 - t, 1.5), p.x), 0.5, p.z);
	return max(1.0 - min(min(length(p - p1), length(p - p2)), length(p - p3)), 0.0);
}

vec3 translucentColor(vec3 p, float t) {
	vec3 color = vec3(0.766, 0.733, 0.813) * 0.5;
	return ( maxcomp(p / 3.0) + maxcomp((3.0 - p) / 3.0) ) * color * sqrt(pulse1(t - 9.6666) + pulse2(0.75 * (t - 32.5)));	
}

vec4 tileColor(vec3 p, float t) {
	p.y = -p.y;
	if (maxcomp(p) > 3.0) return vec4(0.0);
	vec3 uvw = mod(p, 1.0);
	vec3 color = vec3(0.566, 0.533, 0.813);
	float tileBorder = 0.45;
	bool inTile = maxcomp(abs(uvw.xy - 0.5)) < tileBorder;
	inTile = inTile || maxcomp(abs(uvw.xz - 0.5)) < tileBorder;
	inTile = inTile || maxcomp(abs(uvw.yz - 0.5)) < tileBorder;
	
	vec3 tilePos = floor(p) + 0.5;
	vec3 side = vec3(lessThan(p, vec3(0.0000001)));
	vec3 sidePattern = vec3(sideX(tilePos, t - 4.0), sideY(tilePos, t), sideZ(tilePos, t - 9.0)) * float(inTile);

	float tileAlpha = sum(side * sidePattern);
	tileAlpha = min(tileAlpha, 1.0);
	
	vec3 tileColor = mix(translucentColor(p, t + 11.0), color, tileAlpha);
	
	return vec4(tileColor, 1.0);
	
}

float diffuseLit(vec3 p, vec3 n) {
	float result = 0.0;
	n = normalize(n);
	
	vec3 light1 = vec3(0.0, 9.0, 3.0);
	vec3 light1Dir = light1 - p;
	float light1Dist = length(light1Dir);
	result += max(dot(n, normalize(light1Dir)), 0.0);
	
	vec3 light2 = vec3(0.0, 1.5, -14.0);
	vec3 light2Dir = light2 - p;
	float light2Dist = length(light2Dir);
	result += max(dot(n, normalize(light2Dir)), 0.0);
	
	vec3 light3 = vec3(-9.0, -1.5, 2.0);
	vec3 light3Dir = light3 - p;
	float light3Dist = length(light3Dir);
	result += max(dot(n, normalize(light3Dir)), 0.0);
	
	return result;
		
}

vec4 finalLogoColor(vec3 p, vec3 n) {
	n = pow(abs(n), vec3(9001.0));
	p.y = -p.y;
	vec3 color = vec3(0.0);
	color.y = 1.0 - pow(length(p.xz - vec2(2.7, 2.3)), 1.3) / 3.5;	
	color.x = pow(length((p.yz - vec2(3.7, -1.0)) * vec2(1.0, 1.1)), 1.3) / 6.0;
	color.z = pow(length(p.xy - vec2(2.7, 2.3)), 1.3) / 4.0;
	float mixVal = clamp(sum(color * n), 0.0, 1.0);
	vec4 res = mix(vec4(0.98, 0.98, 0.99, 1.0), vec4(0.74, 0.71, 0.95, 1.0), mixVal);
	res.xyz *= sum(n * vec3(0.85, 1.0, 0.6));
	return res;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	float time = mod(iTime - 1.0, 10.0) * 5.0 + 5.0;
	if (iTime == 10.0 && length(iResolution.xy) < 350.0) time = 40.0;
	
	vec2 screenPos = (fragCoord.xy / iResolution.xy) * 2.0 - 1.0;
	screenPos.x *= iResolution.x / iResolution.y;
	screenPos += vec2(0.3, -0.3);
	screenPos = rotate2d(screenPos, 0.25 * pulse1(time - 9.6666));
	screenPos -= vec2(0.3, -0.3);
	screenPos.y -= 0.7;
	vec3 cameraPos = normalize(vec3(-1.0, 1.0 + 2.0 * pulse2(0.75 * (time - 32.5)), -1.0)) * 90.0;
	vec3 cameraDir = normalize(vec3(3.0, 0.0, 3.0) - cameraPos);
	
	vec3 cameraPlaneU = vec3(0.0);
	cameraPlaneU.xz = cross(vec3(0.0, 1.0, 0.0), vec3(cameraDir.x, 0.0, cameraDir.z)).xz;
	cameraPlaneU = normalize(cameraPlaneU);
	vec3 cameraPlaneV = normalize(cross(cameraDir, cameraPlaneU));
	cameraDir *= 30.0;
	
	vec3 rayDir = normalize(cameraDir + screenPos.x * cameraPlaneU + screenPos.y * cameraPlaneV);
	vec3 rayPos = cameraPos;
	
	float dist = scene(rayPos, time);
	int stepsTaken;
	
	vec3 planeDist;
	planeDist.x = cameraPos.x * length(rayDir / rayDir.x);
	planeDist.y = cameraPos.y * length(rayDir / rayDir.y);
	planeDist.z = cameraPos.z * length(rayDir / rayDir.z);
	float planeIntersectDist = -mincomp(abs(planeDist));
	
	for (int i = 0; i < MAX_RAY_STEPS; i++) {
		rayPos += rayDir * dist;
		dist = scene(rayPos, time);
	}
	float material = getMaterial(rayPos, time);
	vec2 dd = vec2(0.0, 1.0) * 0.00001;
	vec3 normal = normalize(vec3(scene(rayPos + dd.yxx, time) - scene(rayPos - dd.yxx, time),
							     scene(rayPos + dd.xyx, time) - scene(rayPos - dd.xyx, time),
							     scene(rayPos + dd.xxy, time) - scene(rayPos - dd.xxy, time)));
	
	vec4 color;
	if (material == 0.0) color = tileColor(rayPos, time - 11.0);
	else color = vec4(0.566, 0.533, 0.863, 1.0) * diffuseLit(rayPos, normal);
	color = clamp(color, 0.0, 1.0);
	
	rayPos = cameraPos;
	
	dist = finalLogoScene(rayPos, time);
	for (int i = 0; i < MAX_RAY_STEPS; i++) {
		if (dist < 0.001) continue;
		rayPos += rayDir * dist;
		dist = finalLogoScene(rayPos, time);
	}
	normal = normalize(vec3(finalLogoScene(rayPos + dd.yxx, time) - finalLogoScene(rayPos - dd.yxx, time),
							     finalLogoScene(rayPos + dd.xyx, time) - finalLogoScene(rayPos - dd.xyx, time),
							     finalLogoScene(rayPos + dd.xxy, time) - finalLogoScene(rayPos - dd.xxy, time)));
	float mixAmount = smoothstep(32.0, 40.0, time);
	if (dist < 0.05)
		color = mix(color, clamp(vec4(vec3(finalLogoColor(rayPos, normal)), 1.0), 0.0, 1.0), mixAmount);
	else
		color = mix(color, vec4(0.0), mixAmount);
	color = mix(color, vec4(0.0), smoothstep(45.0, 50.0, time));
	color = pow(color, vec4(2.2));

	fragColor = color;
}

