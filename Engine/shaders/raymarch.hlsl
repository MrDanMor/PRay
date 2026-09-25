RWTexture2D<float4> Output : register(u0, space1);

static const float3 SphereCenter = float3(0.0f, 0.0f, 3.0f);
static const float SphereRadius = 1.0f;
static const float3 CameraPosition = float3(0.0f, 0.0f, -3.0f);
static const float3 LightDirection = normalize(float3(0.70710678f, 0.70710678f, 0.0f));

float sphereSDF(float3 p) {
    return length(p - SphereCenter) - SphereRadius;
}

float3 estimateNormal(float3 p) {
    return normalize(p - SphereCenter);
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID) {
    uint width, height;
    Output.GetDimensions(width, height);

    if (dispatchId.x >= width || dispatchId.y >= height) {
        return;
    }

    float2 uv = float2(
        (float(dispatchId.x) / max(width, 1u) - 0.5f) * 2.0f,
        (0.5f - float(dispatchId.y) / max(height, 1u)) * 2.0f
    );

    float3 ro = CameraPosition;
    float3 rd = normalize(float3(uv.x, uv.y, 1.5f));

    float t = 0.0f;
    float hitDist = -1.0f;
    const float maxDist = 20.0f;
    const float epsilon = 0.001f;
    const int maxSteps = 128;

    [loop]
    for (int i = 0; i < maxSteps; ++i) {
        float3 p = ro + rd * t;
        float d = sphereSDF(p);

        if (d < epsilon) {
            hitDist = t;
            break;
        }

        if (t > maxDist) {
            break;
        }

        t += max(d * 0.7f, 0.02f);
    }

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (hitDist > 0.0f) {
        float3 hitPoint = ro + rd * hitDist;
        float3 normal = estimateNormal(hitPoint);
        float diffuse = max(0.0f, dot(normal, LightDirection));
        float ambient = 0.25f;
        float3 baseColor = float3(0.5f, 0.5f, 0.5f);
        float3 shaded = baseColor * (ambient + diffuse * 0.9f);
        color = float4(shaded, 1.0f);
    }

    Output[dispatchId.xy] = color;
}