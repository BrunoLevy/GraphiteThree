//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Voxel tyre
// By       Dave Hoskins


#define REFLECTIONS_ON
							
#define PI  3.1415926535

vec3 areaPlane = normalize(vec3(-1.0, 1.0, 1.0));
vec3 lightDir = normalize(vec3(-337.0, 743.0, 330.0));
mat3 rotateMat;
float height;



float deTorus( vec3 p, vec2 t )
{
	p.y -= height;
	p = rotateMat * p;
	vec2 q = vec2(length(p.xz)-t.x,p.y);
	return length(q)-t.y;
}

float deTorusWarped( vec3 p, vec2 t, out int material)
{
	p.y -= height;
	p = rotateMat * p;
	float l = length(p.xz);
	l= pow(l, 4.0) * .00008;
	vec2 q = vec2(l-t.x,p.y);
	if (q.x < -3.5) material = 2;
	else material = 3;
	return length(q)-t.y;
}

float Scene(vec3 p, out int material)
{	
    float d;
	d = deTorusWarped(p, vec2(10.2,8.0), material);
	float d2 = deTorus(p, vec2(13.3, 4.5));
	d = max(d, -d2);
	d = min(d, p.y+4.0);
	
    return d;
}

//----------------------------------------------------------------------------
// Voxel grid search that I found in 1994 in Graphics Gems IV - "Voxel Traversal along a 3D Line"!
// This (Amanatides & Woo) varient is from another shader on here.
float VoxelTrace(vec3 ro, vec3 rd, out bool hit, out vec3 hitNormal, out vec3 pos, out int material)
{
    const int maxSteps = 100;
    vec3 voxel = floor(ro)+.501;
    vec3 step = sign(rd);
	//voxel = voxel + vec3(rd.x > 0.0, rd.y > 0.0, rd.z > 0.0);
    vec3 tMax = (voxel - ro) / rd;
    vec3 tDelta = 1.0 / abs(rd);
    vec3 hitVoxel = voxel;
	int mat = 0;
	
    hit = false;
	
    float hitT = 0.0;
    for(int i=0; i < maxSteps; i++)
	{
		if (!hit)
		{
			float d = Scene(voxel, mat);        
			if (d <= 0.0 && !hit)
			{
				hit = true;
				hitVoxel = voxel;
				material = mat;
                break;
			}
			bool c1 = tMax.x < tMax.y;
			bool c2 = tMax.x < tMax.z;
			bool c3 = tMax.y < tMax.z;
			if (c1 && c2) 
			{ 
				if (!hit) 
				{
					hitNormal = vec3(-step.x, 0.0, 0.0);
					hitT = tMax.x;
				}
				voxel.x += step.x;
				tMax.x += tDelta.x;
	
			} else if (c3 && !c1) 
			{
				if (!hit) 
				{
					hitNormal = vec3(0.0, -step.y, 0.0);	
					hitT = tMax.y;
				}
				voxel.y += step.y;
				tMax.y += tDelta.y;
			} else
			{
				if (!hit) 
				{
					hitNormal = vec3(0.0, 0.0, -step.z);		
					hitT = tMax.z;
				}
				voxel.z += step.z;
				tMax.z += tDelta.z;
			}
		}
    }
	if (hit && (hitVoxel.x > 27.0 || hitVoxel.x < -27.0 || hitVoxel.z < -27.0 || hitVoxel.z > 27.0))
	{
		hit = false;
		return 1000.0;
	}
	
	pos = ro + hitT * rd;
	return hitT;
}

//----------------------------------------------------------------------------
// Do all the ray casting for voxels...
float TraceEverything(vec3 ro, vec3 rd, out int material, out vec3 hitNormal, out vec3 pos)
{
	bool hit1;
	int hit2;
	vec3 pos2;
    float dist = VoxelTrace(ro, rd, hit1, hitNormal, pos, material);
	if (hit1)
	{
		if (pos.y < -3.45)
		{
			material = 1;
		}		
	}else
	{
		material = 0;
	}
	return dist;
}

//----------------------------------------------------------------------------
bool TraceShadow(vec3 ro, vec3 rd)
{
	bool hit;
	vec3 pos;
	vec3 hitNormal;
	int mat;
	float dist2 = VoxelTrace(ro+rd*0.6, rd, hit, hitNormal, pos, mat);
	return hit;
}

//----------------------------------------------------------------------------
vec3 DoMaterialRGB(int m, vec3 pos, vec3 norm, vec3 rd, vec3 ro)
{
	vec3 rgb;
	float diff = dot(norm, lightDir);
    diff = max(diff, 0.0);
    //return diff *areaColor;	
	if (m == 1)
	{
		rgb = diff * vec3(0.1, 0.1, 0.1);
		
	}else
	if (m == 2)
	{
		rgb = diff * vec3(1.0, 1.0, 1.0);
		
	}else
	if (m == 3)
	{
		rgb = diff * vec3(0.2, 0.2, 0.2);
		
	}else
	{
		rgb = mix(vec3(.0, .05, .1), vec3(0.4, 0.3, .6), abs(sin(rd.y*3.0)));
    }
	return rgb;
}

//----------------------------------------------------------------------------
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 pixel = (fragCoord.xy / iResolution.xy)*2.0-1.0;
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp*pixel.x, pixel.y-1.3, -3.0));
    vec3 ro = vec3(0.0, 50.0, 90.0);
	mat3 matZ, matY;

    float time = iTime * 3.1+230.0;

	time = (cos(time / PI) + 1.0);
	// Rotate Z
	float z = time * .8;
	float sz = sin(z);
	float cz = cos(z);
	matZ[0] = vec3(cz,		-sz,	0.0);
	matZ[1] = vec3(sz,		cz,		0.0);
	matZ[2] = vec3(0.0,	0.0,	1.0);
	height = (sz) *  14.0+4.0;
	
	// Rotate Y
	float y = time * 4.0*PI + PI*.5;
	float sy = sin(y);
	float cy = cos(y);
	matY[0] = vec3(cy,		0.0,	-sy);
	matY[1] = vec3(0.0,		1.0,	0.0);
	matY[2] = vec3(sy,		0.0,	 cy);
	
	rotateMat = matZ * matY;

	vec3 rgb;
    vec3 norm, pos;
	int material; 

	TraceEverything(ro+rd*70.0, rd, material, norm, pos);
	rgb = DoMaterialRGB(material, pos, norm, rd, ro);
	
	// Do the shadow casting...
	if (material > 0 && TraceShadow(pos+lightDir*.04, lightDir) == true)
	{
		rgb *= .35;
	}
	
		
#ifdef REFLECTIONS_ON
	if (material > 0 && material != 3)
	{
		ro = pos;
		rd = ((-2.0*(dot(rd, norm))*norm)+rd);
		TraceEverything(ro+rd*0.04, rd, material, norm, pos);
		rgb = mix(rgb, DoMaterialRGB(material, pos, norm, rd, ro), .2);
	}
#endif

	// Curve the brightness a little...
	rgb = pow(rgb, vec3(.65, .65, .65));
    fragColor=vec4(rgb, 1.0);
}
							   
							   
