// struct SphereData {
//     float4 centerRadius;
//     float4 lightDirPad;
// };

// struct CameraData {
//     float4 position;
//     float4 movement;
//     float4 rotationAspect;
// };

// ConstantBuffer<SphereData> Sphere : register(b0);
// ConstantBuffer<CameraData> Camera : register(b1);

// RWTexture2D<float4> Output : register(u0);

// float sphereSDF(float3 p) {
//     return length(p - Sphere.centerRadius.xyz) - Sphere.centerRadius.w;
// }

// float3 estimateNormal(float3 p) {
//     return normalize(p - Sphere.centerRadius.xyz);
// }

// [numthreads(16, 16, 1)]
// void main(uint3 dispatchId : SV_DispatchThreadID) {
//     uint width, height;
//     Output.GetDimensions(width, height);

//     if (dispatchId.x >= width || dispatchId.y >= height) {
//         return;
//     }

//     float2 uv = float2(
//         (2.0f * dispatchId.x / max(width, 1u) - 1.0f) * Camera.rotationAspect.z,
//         1.0f - 2.0f * dispatchId.y / max(height, 1u)
//     );

//     float3 ro = Camera.position.xyz;
//     float3 rd = normalize(float3(uv.x, uv.y, 1.0f));

//     float t = 0.0f;
//     float hitDist = -1.0f;
//     const float maxDist = 100.0f;
//     const float epsilon = 0.001f;
//     const int maxSteps = 256;

//     for (int i = 0; i < maxSteps; ++i) {
//         float3 p = ro + rd * t;
//         float d = sphereSDF(p);

//         if (d < epsilon) {
//             hitDist = t;
//             break;
//         }

//         if (t > maxDist) {
//             break;
//         }

//         t += d;
//     }

//     float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

//     if (hitDist > 0.0f) {
//         float3 hitPoint = ro + rd * hitDist;
//         float3 normal = estimateNormal(hitPoint);
//         float ambient = 0.2f;
//         float diffuse = max(0.0f, dot(normal, normalize(Sphere.lightDirPad.xyz)));
//         float brightness = min(ambient + diffuse * 0.8f, 1.0f);
//         color = float4(brightness, brightness, brightness, 1.0f);
//     }

//     Output[dispatchId.xy] = color;
// }