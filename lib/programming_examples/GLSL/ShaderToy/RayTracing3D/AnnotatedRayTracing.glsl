//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Ray tracing is a topic I have always wanted to explore, but never really had
// the opportunity to do so until now. What exactly is ray tracing? Consider a
// lamp hanging from the ceiling. Light is constantly being emitted from the
// lamp in the form of light rays, which bounce around the room until they hit
// your eye. Ray tracing follows a similar concept by simulating the path of
// light through a scene, except in reverse. There is no point in doing the math
// for light rays you cannot see!

// Algorithmically, ray tracing is very elegant. For each pixel, shoot a light
// ray from the camera through each pixel on screen. If the ray collides with
// geometry in the scene, create new rays that perform the same process for both
// reflection, as in a mirror, and refraction, as in through water. Repeat
// to your satisfaction.

// Having worked extensively with OpenCL in the past, this seemed like a good
// candidate to port to a parallel runtime on a GPU. Inspired by the [smallpt](http://www.kevinbeason.com/smallpt/#moreinfo)
// line-by-line explanation, I decided to write a parallel ray tracer with
// extensive annotations. The results are below ...

// ![screenshot](/uploads/raytracer.png)

// I start with a simple ray definition, consisting of an origin point and a
// direction vector. I also define a directional light to illuminate my scene.
struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Light {
    vec3 color;
    vec3 direction;
};

// In real life, objects have many different material properties. Some objects
// respond very differently to light than others. For instance, a sheet of paper
// and a polished mirror. The former exhibits a strong *diffuse* response;
// incoming light is reflected at many angles. The latter is an example of a
// *specular* response, where incoming light is reflected in a single direction.
// To model this, I create a basic material definition. Objects in my scene
// share a single (RGB) color with diffuse and specular weights.
struct Material {
    vec3 color;
    float diffuse;
    float specular;
};

// To render the scene, I need to know where a ray intersects with an object.
// Since rays have infinite length from an origin, I can model the point of
// intersection by storing the distance along the ray. I also need to store the
// surface normal so I know which way to bounce! Once I create a ray, it loses
// the concept of scene geometry, so one more thing I do is forward the surface
// material properties.
struct Intersect {
    float len;
    vec3 normal;
    Material material;
};

// The last data structures I create are for objects used to fill my scene. The
// most basic object I can model is a sphere, which is defined as a radius at
// some center position, with some material properties. To draw the floor, I
// also define a simple horizontal plane centered at the origin, with a normal
// vector pointing upwards.
struct Sphere {
    float radius;
    vec3 position;
    Material material;
};

struct Plane {
    vec3 normal;
    Material material;
};

// At this point, I define some global variables. A more advanced program might
// pass these values in as uniforms, but for now, this is easier to tinker with.
// Due to floating point precision errors, when a ray intersects geometry at a
// surface, the point of intersection could possibly be just below the surface.
// The subsequent reflection ray would then bounce off the *inside* wall of the
// surface. This is known as self-intersection. When creating new rays, I
// initialize them at a slightly offset origin to help mitigate this problem.
const float epsilon = 1e-3;

// The classical ray tracing algorithm is recursive. However, GLSL does not
// support recursion, so I instead use an iterative approach to control the
// number of light bounces.
const int iterations = 16;

// Next, I define an exposure time and gamma value. At this point, I also create
// a basic directional light and define the ambient light color; the color here
// is mostly a matter of taste. Basically ... lighting controls.
const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;

// For a Static Light
Light light = Light(vec3(1.0) * intensity, normalize(vec3(-1.0, 0.75, 1.0)));

// For a Rotating Light
// Light light = Light(vec3(1.0) * intensity, normalize(
//                vec3(-1.0 + 4.0 * cos(iTime), 4.75,
//                      1.0 + 4.0 * sin(iTime))));

// I strongly dislike this line. I needed to know when a ray hits or misses a
// surface. If it hits geometry, I returned the point at the surface. Otherwise,
// the ray misses all geometry and instead hits the sky box. In a language that
// supports dynamic return values, I could `return false`, but that is not an
// option in GLSL. In the interests of making progress, I created an intersect
// of distance zero to represent a miss and moved on.
const Intersect miss = Intersect(0.0, vec3(0.0), Material(vec3(0.0), 0.0, 0.0));

// As indicated earlier, I implement ray tracing for spheres. I need to compute
// the point at which a ray intersects with a sphere. [Line-Sphere](http://en.wikipedia.org/wiki/Line-sphere_intersection)
// intersection is relatively straightforward. For reflection purposes, a ray
// either hits or misses, so I need to check for no solutions, or two solutions.
// In the latter case, I need to determine which solution is "in front" so I can
// return an intersection of appropriate distance from the ray origin.
Intersect intersect(Ray ray, Sphere sphere) {
    // Check for a Negative Square Root
    vec3 oc = sphere.position - ray.origin;
    float l = dot(ray.direction, oc);
    float det = pow(l, 2.0) - dot(oc, oc) + pow(sphere.radius, 2.0);
    if (det < 0.0) return miss;

    // Find the Closer of Two Solutions
             float len = l - sqrt(det);
    if (len < 0.0) len = l + sqrt(det);
    if (len < 0.0) return miss;
    return Intersect(len, (ray.origin + len*ray.direction - sphere.position) / sphere.radius, sphere.material);
}

// Since I created a floor plane, I likewise have to handle reflections for
// planes by implementing [Line-Plane](http://en.wikipedia.org/wiki/Line-plane_intersection)
// intersection. I only care about the intersect for the purposes of reflection,
// so I only check if the quotient is non-zero.
Intersect intersect(Ray ray, Plane plane) {
    float len = -dot(ray.origin, plane.normal) / dot(ray.direction, plane.normal);
    if (len < 0.0) return miss;
    return Intersect(len, plane.normal, plane.material);
}

// In a *real* ray tracing renderer, geometry would be passed in from the host
// as a mesh containing vertices, normals, and texture coordinates, but for the
// sake of simplicity, I hand-coded the scene-graph. In this function, I take an
// input ray and iterate through all geometry to determine intersections.
Intersect trace(Ray ray) {
    const int num_spheres = 3;
    Sphere spheres[num_spheres];

    // I initially started with the [smallpt](www.kevinbeason.com/smallpt/)
    // scene definition, but soon found performance was abysmal on very large
    // spheres. I kept the general format, modified to fit my data structures.

    spheres[0] = Sphere(2.0, vec3(-4.0, 3.0 + sin(iTime), 0), Material(vec3(1.0, 0.0, 0.2), 1.0, 0.001));
    spheres[1] = Sphere(3.0, vec3( 4.0 + cos(iTime), 3.0, 0), Material(vec3(0.0, 0.2, 1.0), 1.0, 0.0));
    spheres[2] = Sphere(1.0, vec3( 0.5, 1.0, 6.0),                  Material(vec3(1.0, 1.0, 1.0), 0.5, 0.25));

    // Since my ray tracing approach involves drawing to a 2D quad, I can no
    // longer use the OpenGL Depth and Stencil buffers to control the draw
    // order. Drawing is therefore sensitive to z-indexing, so I first intersect
    // with the plane, then loop through all spheres back-to-front.

    Intersect intersection = miss;
    Intersect plane = intersect(ray, Plane(vec3(0, 1, 0), Material(vec3(1.0, 1.0, 1.0), 1.0, 0.0)));
    if (plane.material.diffuse > 0.0 || plane.material.specular > 0.0) { intersection = plane; }
    for (int i = 0; i < num_spheres; i++) {
        Intersect sphere = intersect(ray, spheres[i]);
        if (sphere.material.diffuse > 0.0 || sphere.material.specular > 0.0)
            intersection = sphere;
    }
    return intersection;
}

// This is the critical part of writing a ray tracer. I start with some empty
// scratch vectors for color data and the Fresnel factor. I trace the scene with
// using an input ray, and continue to fire new rays until the iteration depth
// is reached, at which point I return the total sum of the color values from
// computed at each bounce.
vec3 radiance(Ray ray) {
    vec3 color = vec3(0.0), fresnel = vec3(0.0);
    vec3 mask = vec3(1.0);
    for (int i = 0; i <= iterations; ++i) {
        Intersect hit = trace(ray);

        // This goes back to the dummy "miss" intersect. Basically, if the scene
        // trace returns an intersection with either a diffuse or specular
        // coefficient, then it has encountered a surface of a sphere or plane.
        // Otherwise, the current ray has reached the ambient-colored sky box.

        if (hit.material.diffuse > 0.0 || hit.material.specular > 0.0) {

            // Here I use the [Schlick Approximation](http://en.wikipedia.org/wiki/Schlick's_approximation)
            // to determine the Fresnel specular contribution factor, a measure
            // of how much incoming light is reflected or refracted. I compute
            // the Fresnel term and use a mask to track the fraction of
            // reflected light in the current ray with respect to the original.

            vec3 r0 = hit.material.color.rgb * hit.material.specular;
            float hv = clamp(dot(hit.normal, -ray.direction), 0.0, 1.0);
            fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0);
            mask *= fresnel;

            // I handle shadows and diffuse colors next. I condensed this part
            // into one conditional evaluation for brevity. Remember `epsilon`?
            // I use it to trace a ray slightly offset from the point of
            // intersection to the light source. If the shadow ray does not hit
            // an object, it will be a "miss" as it hits the skybox. This means
            // there are no objects between the point and the light, at which
            // point I can add the diffuse color to the fragment color since the
            // object is not in shadow.

            if (trace(Ray(ray.origin + hit.len * ray.direction + epsilon * light.direction, light.direction)) == miss) {
                color += clamp(dot(hit.normal, light.direction), 0.0, 1.0) * light.color
                       * hit.material.color.rgb * hit.material.diffuse
                       * (1.0 - fresnel) * mask / fresnel;
            }

            // After computing diffuse colors, I then generate a new reflection
            // ray and overwrite the original ray that was passed in as an
            // argument to the radiance(...) function. Then I repeat until I
            // reach the iteration depth.

            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);

        } else {

            // This is the other half of the tracing branch. If the trace failed
            // to return an intersection with an attached material, then it is
            // safe to assume that the ray points at the sky, or out of bounds
            // of the scene. At this point I realized that real objects have a
            // small sheen to them, so I hard-coded a small spotlight pointing
            // in the same direction as the main light for pseudo-realism.

            vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 250.0);
            color += mask * (ambient + spotlight); break;
        }
    }
    return color;
}

// The main function primarily deals with organizing data from OpenGL into a
// format that the ray tracer can use. For ray tracing, I need to fire a ray for
// each pixel, or more precisely, a ray for every fragment. However, pixels to
// fragment coordinates do not map one a one-to-one basis, so I need to divide
// the fragment coordinates by the viewport resolution. I then offset that by a
// fixed value to re-center the coordinate system.

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv    = fragCoord.xy / iResolution.xy - vec2(0.5);
         uv.x *= iResolution.x / iResolution.y;

    // For each fragment, create a ray at a fixed point of origin directed at
    // the coordinates of each fragment. The last thing before writing the color
    // to the fragment is to post-process the pixel values using tone-mapping.
    // In this case, I adjust for exposure and perform linear gamma correction.

    Ray ray = Ray(vec3(0.0, 2.5, 12.0), normalize(vec3(uv.x, uv.y, -1.0)));
    fragColor = vec4(pow(radiance(ray) * exposure, vec3(1.0 / gamma)), 1.0);
}

// If all goes well, you should see an animated scene below!
// <iframe src="https://www.shadertoy.com/embed/4ljGRd?gui=true&paused=false"
//         width="100%" height="380px" frameborder="0" allowfullscreen></iframe>

// This was my first foray into ray tracing. Originally, I wanted to write this
// using the OpenGL Compute Shader. That was harder to setup than I originally
// realized, and I spent a fair bit of time mucking around with OpenGL and cmake
// before deciding to just sit down and start programming.

// All things considered, this is a pretty limited ray tracer. Some low hanging
// fruit might be to add anti-aliasing and soft shadows. The former was not an
// issue until I ported this from a HiDPI display onto the WebGL canvas. The
// latter involves finding a quality random number generator. Maybe a summer
// project before I start working ...

