#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer TransformBuffer
// {
//
//   struct TransformMatrices
//   {
//       
//       float4x4 model;                // Offset:    0
//       float4x4 view;                 // Offset:   64
//       float4x4 projection;           // Offset:  128
//       float4x4 modelView;            // Offset:  192
//       float4x4 viewProjection;       // Offset:  256
//       float4x4 modelViewProjection;  // Offset:  320
//
//   } Transforms;                      // Offset:    0 Size:   384
//
// }
//
// cbuffer LightBuffer
// {
//
//   struct DirectionalLight
//   {
//       
//       float4 direction;              // Offset:    0
//       float4 color;                  // Offset:   16
//       float intensity;               // Offset:   32
//       float3 _pad1;                  // Offset:   36
//       bool castsShadow;              // Offset:   48
//       bool enabled;                  // Offset:   52
//       float2 _pad2;                  // Offset:   56
//       float4x4 lightViewProj;        // Offset:   64
//
//   } Sun;                             // Offset:    0 Size:   128
//   
//   struct PointLight
//   {
//       
//       float3 position;               // Offset:  128
//       float padding1;                // Offset:  140
//       float3 color;                  // Offset:  144
//       float intensity;               // Offset:  156
//       float constant;                // Offset:  160
//       float lin;                     // Offset:  164
//       float quadratic;               // Offset:  168
//       float radius;                  // Offset:  172
//       bool castsShadow;              // Offset:  176
//       bool enabled;                  // Offset:  180
//       float2 padding2;               // Offset:  184
//
//   } PointLights[16];                 // Offset:  128 Size:  1024 [unused]
//   
//   struct SpotLight
//   {
//       
//       float3 position;               // Offset: 1152
//       float _padding1;               // Offset: 1164
//       float3 direction;              // Offset: 1168
//       float _padding2;               // Offset: 1180
//       float3 color;                  // Offset: 1184
//       float intensity;               // Offset: 1196
//       float innerAngle;              // Offset: 1200
//       float outerAngle;              // Offset: 1204
//       float range;                   // Offset: 1208
//       bool castsShadow;              // Offset: 1212
//       bool enabled;                  // Offset: 1216
//       float3 _pad;                   // Offset: 1220
//
//   } SpotLights[16];                  // Offset: 1152 Size:  1280 [unused]
//   
//   struct AreaLight
//   {
//       
//       float3 position;               // Offset: 2432
//       float _pad1;                   // Offset: 2444
//       float3 direction;              // Offset: 2448
//       float _pad2;                   // Offset: 2460
//       float3 color;                  // Offset: 2464
//       float _pad3;                   // Offset: 2476
//       float2 dimensions;             // Offset: 2480
//       float intensity;               // Offset: 2488
//       bool castsShadow;              // Offset: 2492
//       bool enabled;                  // Offset: 2496
//       float3 _pad;                   // Offset: 2500
//
//   } AreaLights[16];                  // Offset: 2432 Size:  1280 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// TransformBuffer                   cbuffer      NA          NA            cb0      1 
// LightBuffer                       cbuffer      NA          NA            cb1      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float   xyz 
// TANGENT                  0   xyz         2     NONE   float   xyz 
// TEXCOORD                 0   xy          3     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xy          1     NONE   float   xy  
// TEXCOORD                 1     zw        1     NONE   float       
// NORMAL                   0   xyz         2     NONE   float   xyz 
// TANGENT                  0   xyz         3     NONE   float   xyz 
// BINORMAL                 0   xyz         4     NONE   float   xyz 
// TEXCOORD                 2   xyz         5     NONE   float   xyz 
// TEXCOORD                 3   xyzw        6     NONE   float   xyzw
// TEXCOORD                 4   xyzw        7     NONE   float   xyzw
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[24], immediateIndexed
dcl_constantbuffer CB1[8], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyz
dcl_input v2.xyz
dcl_input v3.xy
dcl_output_siv o0.xyzw, position
dcl_output o1.xy
dcl_output o2.xyz
dcl_output o3.xyz
dcl_output o4.xyz
dcl_output o5.xyz
dcl_output o6.xyzw
dcl_output o7.xyzw
dcl_temps 4
mov r0.xyz, v0.xyzx
mov r0.w, l(1.000000)
dp4 r1.x, r0.xyzw, cb0[20].xyzw
dp4 r1.y, r0.xyzw, cb0[21].xyzw
dp4 r1.z, r0.xyzw, cb0[22].xyzw
dp4 r1.w, r0.xyzw, cb0[23].xyzw
mov o0.xyzw, r1.xyzw
mov o6.xyzw, r1.xyzw
mov o1.xy, v3.xyxx
dp3 r1.x, v1.xyzx, cb0[0].xyzx
dp3 r1.y, v1.xyzx, cb0[1].xyzx
dp3 r1.z, v1.xyzx, cb0[2].xyzx
dp3 r1.w, r1.xyzx, r1.xyzx
rsq r1.w, r1.w
mul r1.xyz, r1.wwww, r1.xyzx
mov o2.xyz, r1.xyzx
dp3 r2.x, v2.xyzx, cb0[0].xyzx
dp3 r2.y, v2.xyzx, cb0[1].xyzx
dp3 r2.z, v2.xyzx, cb0[2].xyzx
dp3 r1.w, r2.xyzx, r2.xyzx
rsq r1.w, r1.w
mul r2.xyz, r1.wwww, r2.xyzx
mov o3.xyz, r2.xyzx
mul r3.xyz, r1.zxyz, r2.yzxy
mad o4.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
dp4 r1.x, r0.xyzw, cb0[0].xyzw
dp4 r1.y, r0.xyzw, cb0[1].xyzw
dp4 r1.z, r0.xyzw, cb0[2].xyzw
dp4 r1.w, r0.xyzw, cb0[3].xyzw
mov o5.xyz, r1.xyzx
dp4 o7.x, r1.xyzw, cb1[4].xyzw
dp4 o7.y, r1.xyzw, cb1[5].xyzw
dp4 o7.z, r1.xyzw, cb1[6].xyzw
dp4 o7.w, r1.xyzw, cb1[7].xyzw
ret 
// Approximately 35 instruction slots used
#endif

const BYTE kWater_VSBytes[] =
{
     68,  88,  66,  67,  15, 190, 
     22, 215,  24, 192, 230, 145, 
    252,  51,  50,  37, 204,  13, 
    108,  16,   1,   0,   0,   0, 
     84,  14,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    124,   7,   0,   0,  16,   8, 
      0,   0,  40,   9,   0,   0, 
    184,  13,   0,   0,  82,  68, 
     69,  70,  64,   7,   0,   0, 
      2,   0,   0,   0, 152,   0, 
      0,   0,   2,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    254, 255,   0,   1,   0,   0, 
     24,   7,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    124,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 140,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  84, 114, 
     97, 110, 115, 102, 111, 114, 
    109,  66, 117, 102, 102, 101, 
    114,   0,  76, 105, 103, 104, 
    116,  66, 117, 102, 102, 101, 
    114,   0, 124,   0,   0,   0, 
      1,   0,   0,   0, 200,   0, 
      0,   0, 128,   1,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 140,   0,   0,   0, 
      4,   0,   0,   0, 236,   1, 
      0,   0, 128,  14,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 240,   0,   0,   0, 
      0,   0,   0,   0, 128,   1, 
      0,   0,   2,   0,   0,   0, 
    200,   1,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     84, 114,  97, 110, 115, 102, 
    111, 114, 109, 115,   0,  84, 
    114,  97, 110, 115, 102, 111, 
    114, 109,  77,  97, 116, 114, 
    105,  99, 101, 115,   0, 109, 
    111, 100, 101, 108,   0, 102, 
    108, 111,  97, 116,  52, 120, 
     52,   0,   3,   0,   3,   0, 
      4,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  19,   1, 
      0,   0, 118, 105, 101, 119, 
      0, 112, 114, 111, 106, 101, 
     99, 116, 105, 111, 110,   0, 
    109, 111, 100, 101, 108,  86, 
    105, 101, 119,   0, 118, 105, 
    101, 119,  80, 114, 111, 106, 
    101,  99, 116, 105, 111, 110, 
      0, 109, 111, 100, 101, 108, 
     86, 105, 101, 119,  80, 114, 
    111, 106, 101,  99, 116, 105, 
    111, 110,   0, 171, 171, 171, 
     13,   1,   0,   0,  28,   1, 
      0,   0,   0,   0,   0,   0, 
     64,   1,   0,   0,  28,   1, 
      0,   0,  64,   0,   0,   0, 
     69,   1,   0,   0,  28,   1, 
      0,   0, 128,   0,   0,   0, 
     80,   1,   0,   0,  28,   1, 
      0,   0, 192,   0,   0,   0, 
     90,   1,   0,   0,  28,   1, 
      0,   0,   0,   1,   0,   0, 
    105,   1,   0,   0,  28,   1, 
      0,   0,  64,   1,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     96,   0,   0,   0,   6,   0, 
    128,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 251,   0,   0,   0, 
    140,   2,   0,   0,   0,   0, 
      0,   0, 128,   0,   0,   0, 
      2,   0,   0,   0,  44,   4, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  80,   4, 
      0,   0, 128,   0,   0,   0, 
      0,   4,   0,   0,   0,   0, 
      0,   0,  36,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  72,   5,   0,   0, 
    128,   4,   0,   0,   0,   5, 
      0,   0,   0,   0,   0,   0, 
     36,   6,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     72,   6,   0,   0, 128,   9, 
      0,   0,   0,   5,   0,   0, 
      0,   0,   0,   0, 244,   6, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  83, 117, 
    110,   0,  68, 105, 114, 101, 
     99, 116, 105, 111, 110,  97, 
    108,  76, 105, 103, 104, 116, 
      0, 100, 105, 114, 101,  99, 
    116, 105, 111, 110,   0, 102, 
    108, 111,  97, 116,  52,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 171,   2, 
      0,   0,  99, 111, 108, 111, 
    114,   0, 105, 110, 116, 101, 
    110, 115, 105, 116, 121,   0, 
    102, 108, 111,  97, 116,   0, 
    171, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 232,   2, 
      0,   0,  95, 112,  97, 100, 
     49,   0, 102, 108, 111,  97, 
    116,  51,   0, 171, 171, 171, 
      1,   0,   3,   0,   1,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  26,   3,   0,   0, 
     99,  97, 115, 116, 115,  83, 
    104,  97, 100, 111, 119,   0, 
     98, 111, 111, 108,   0, 171, 
    171, 171,   0,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  84,   3, 
      0,   0, 101, 110,  97,  98, 
    108, 101, 100,   0,  95, 112, 
     97, 100,  50,   0, 102, 108, 
    111,  97, 116,  50,   0, 171, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 142,   3, 
      0,   0, 108, 105, 103, 104, 
    116,  86, 105, 101, 119,  80, 
    114, 111, 106,   0, 171, 171, 
    161,   2,   0,   0, 180,   2, 
      0,   0,   0,   0,   0,   0, 
    216,   2,   0,   0, 180,   2, 
      0,   0,  16,   0,   0,   0, 
    222,   2,   0,   0, 240,   2, 
      0,   0,  32,   0,   0,   0, 
     20,   3,   0,   0,  36,   3, 
      0,   0,  36,   0,   0,   0, 
     72,   3,   0,   0,  92,   3, 
      0,   0,  48,   0,   0,   0, 
    128,   3,   0,   0,  92,   3, 
      0,   0,  52,   0,   0,   0, 
    136,   3,   0,   0, 152,   3, 
      0,   0,  56,   0,   0,   0, 
    188,   3,   0,   0,  28,   1, 
      0,   0,  64,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     32,   0,   0,   0,   8,   0, 
    204,   3,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 144,   2,   0,   0, 
     80, 111, 105, 110, 116,  76, 
    105, 103, 104, 116, 115,   0, 
     80, 111, 105, 110, 116,  76, 
    105, 103, 104, 116,   0, 112, 
    111, 115, 105, 116, 105, 111, 
    110,   0, 112,  97, 100, 100, 
    105, 110, 103,  49,   0,  99, 
    111, 110, 115, 116,  97, 110, 
    116,   0, 108, 105, 110,   0, 
    113, 117,  97, 100, 114,  97, 
    116, 105,  99,   0, 114,  97, 
    100, 105, 117, 115,   0, 112, 
     97, 100, 100, 105, 110, 103, 
     50,   0, 103,   4,   0,   0, 
     36,   3,   0,   0,   0,   0, 
      0,   0, 112,   4,   0,   0, 
    240,   2,   0,   0,  12,   0, 
      0,   0, 216,   2,   0,   0, 
     36,   3,   0,   0,  16,   0, 
      0,   0, 222,   2,   0,   0, 
    240,   2,   0,   0,  28,   0, 
      0,   0, 121,   4,   0,   0, 
    240,   2,   0,   0,  32,   0, 
      0,   0, 130,   4,   0,   0, 
    240,   2,   0,   0,  36,   0, 
      0,   0, 134,   4,   0,   0, 
    240,   2,   0,   0,  40,   0, 
      0,   0, 144,   4,   0,   0, 
    240,   2,   0,   0,  44,   0, 
      0,   0,  72,   3,   0,   0, 
     92,   3,   0,   0,  48,   0, 
      0,   0, 128,   3,   0,   0, 
     92,   3,   0,   0,  52,   0, 
      0,   0, 151,   4,   0,   0, 
    152,   3,   0,   0,  56,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,  16,   0,  16,   0, 
     11,   0, 160,   4,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  92,   4, 
      0,   0,  83, 112, 111, 116, 
     76, 105, 103, 104, 116, 115, 
      0,  83, 112, 111, 116,  76, 
    105, 103, 104, 116,   0,  95, 
    112,  97, 100, 100, 105, 110, 
    103,  49,   0,  95, 112,  97, 
    100, 100, 105, 110, 103,  50, 
      0, 105, 110, 110, 101, 114, 
     65, 110, 103, 108, 101,   0, 
    111, 117, 116, 101, 114,  65, 
    110, 103, 108, 101,   0, 114, 
     97, 110, 103, 101,   0,  95, 
    112,  97, 100,   0, 171, 171, 
    103,   4,   0,   0,  36,   3, 
      0,   0,   0,   0,   0,   0, 
     93,   5,   0,   0, 240,   2, 
      0,   0,  12,   0,   0,   0, 
    161,   2,   0,   0,  36,   3, 
      0,   0,  16,   0,   0,   0, 
    103,   5,   0,   0, 240,   2, 
      0,   0,  28,   0,   0,   0, 
    216,   2,   0,   0,  36,   3, 
      0,   0,  32,   0,   0,   0, 
    222,   2,   0,   0, 240,   2, 
      0,   0,  44,   0,   0,   0, 
    113,   5,   0,   0, 240,   2, 
      0,   0,  48,   0,   0,   0, 
    124,   5,   0,   0, 240,   2, 
      0,   0,  52,   0,   0,   0, 
    135,   5,   0,   0, 240,   2, 
      0,   0,  56,   0,   0,   0, 
     72,   3,   0,   0,  92,   3, 
      0,   0,  60,   0,   0,   0, 
    128,   3,   0,   0,  92,   3, 
      0,   0,  64,   0,   0,   0, 
    141,   5,   0,   0,  36,   3, 
      0,   0,  68,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     20,   0,  16,   0,  12,   0, 
    148,   5,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  83,   5,   0,   0, 
     65, 114, 101,  97,  76, 105, 
    103, 104, 116, 115,   0,  65, 
    114, 101,  97,  76, 105, 103, 
    104, 116,   0,  95, 112,  97, 
    100,  51,   0, 100, 105, 109, 
    101, 110, 115, 105, 111, 110, 
    115,   0, 171, 171, 103,   4, 
      0,   0,  36,   3,   0,   0, 
      0,   0,   0,   0,  20,   3, 
      0,   0, 240,   2,   0,   0, 
     12,   0,   0,   0, 161,   2, 
      0,   0,  36,   3,   0,   0, 
     16,   0,   0,   0, 136,   3, 
      0,   0, 240,   2,   0,   0, 
     28,   0,   0,   0, 216,   2, 
      0,   0,  36,   3,   0,   0, 
     32,   0,   0,   0,  93,   6, 
      0,   0, 240,   2,   0,   0, 
     44,   0,   0,   0,  99,   6, 
      0,   0, 152,   3,   0,   0, 
     48,   0,   0,   0, 222,   2, 
      0,   0, 240,   2,   0,   0, 
     56,   0,   0,   0,  72,   3, 
      0,   0,  92,   3,   0,   0, 
     60,   0,   0,   0, 128,   3, 
      0,   0,  92,   3,   0,   0, 
     64,   0,   0,   0, 141,   5, 
      0,   0,  36,   3,   0,   0, 
     68,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,  20,   0, 
     16,   0,  11,   0, 112,   6, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     83,   6,   0,   0,  77, 105, 
     99, 114, 111, 115, 111, 102, 
    116,  32,  40,  82,  41,  32, 
     72,  76,  83,  76,  32,  83, 
    104,  97, 100, 101, 114,  32, 
     67, 111, 109, 112, 105, 108, 
    101, 114,  32,  49,  48,  46, 
     49,   0,  73,  83,  71,  78, 
    140,   0,   0,   0,   4,   0, 
      0,   0,   8,   0,   0,   0, 
    104,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   7,   7,   0,   0, 
    113,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   7,   7,   0,   0, 
    120,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,   7,   7,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,   3,   3,   0,   0, 
     80,  79,  83,  73,  84,  73, 
     79,  78,   0,  78,  79,  82, 
     77,  65,  76,   0,  84,  65, 
     78,  71,  69,  78,  84,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171, 171, 171, 
     79,  83,  71,  78,  16,   1, 
      0,   0,   9,   0,   0,   0, 
      8,   0,   0,   0, 224,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0, 236,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      3,  12,   0,   0, 236,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     12,  15,   0,   0, 245,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
      7,   8,   0,   0, 252,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      7,   8,   0,   0,   4,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
      7,   8,   0,   0, 236,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
      7,   8,   0,   0, 236,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
     15,   0,   0,   0, 236,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   7,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  80,  79,  83,  73,  84, 
     73,  79,  78,   0,  84,  69, 
     88,  67,  79,  79,  82,  68, 
      0,  78,  79,  82,  77,  65, 
     76,   0,  84,  65,  78,  71, 
     69,  78,  84,   0,  66,  73, 
     78,  79,  82,  77,  65,  76, 
      0, 171, 171, 171,  83,  72, 
     69,  88, 136,   4,   0,   0, 
     80,   0,   1,   0,  34,   1, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     24,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   0,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   1,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   2,   0, 
      0,   0,  95,   0,   0,   3, 
     50,  16,  16,   0,   3,   0, 
      0,   0, 103,   0,   0,   4, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3,  50,  32, 
     16,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 114,  32, 
     16,   0,   2,   0,   0,   0, 
    101,   0,   0,   3, 114,  32, 
     16,   0,   3,   0,   0,   0, 
    101,   0,   0,   3, 114,  32, 
     16,   0,   4,   0,   0,   0, 
    101,   0,   0,   3, 114,  32, 
     16,   0,   5,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   6,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   7,   0,   0,   0, 
    104,   0,   0,   2,   4,   0, 
      0,   0,  54,   0,   0,   5, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,  18,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     17,   0,   0,   8,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  20,   0, 
      0,   0,  17,   0,   0,   8, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     21,   0,   0,   0,  17,   0, 
      0,   8,  66,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  22,   0,   0,   0, 
     17,   0,   0,   8, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  23,   0, 
      0,   0,  54,   0,   0,   5, 
    242,  32,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   5, 242,  32,  16,   0, 
      6,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5,  50,  32, 
     16,   0,   1,   0,   0,   0, 
     70,  16,  16,   0,   3,   0, 
      0,   0,  16,   0,   0,   8, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  70,  18,  16,   0, 
      1,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   8,  34,   0,  16,   0, 
      1,   0,   0,   0,  70,  18, 
     16,   0,   1,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     16,   0,   0,   8,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  18,  16,   0,   1,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  16,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     68,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 246,  15,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5, 114,  32, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  16,   0,   0,   8, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  70,  18,  16,   0, 
      2,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   8,  34,   0,  16,   0, 
      2,   0,   0,   0,  70,  18, 
     16,   0,   2,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     16,   0,   0,   8,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  18,  16,   0,   2,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  16,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      2,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     68,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    114,   0,  16,   0,   2,   0, 
      0,   0, 246,  15,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5, 114,  32, 
     16,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  56,   0,   0,   7, 
    114,   0,  16,   0,   3,   0, 
      0,   0,  38,   9,  16,   0, 
      1,   0,   0,   0, 150,   4, 
     16,   0,   2,   0,   0,   0, 
     50,   0,   0,  10, 114,  32, 
     16,   0,   4,   0,   0,   0, 
    150,   4,  16,   0,   1,   0, 
      0,   0,  38,   9,  16,   0, 
      2,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      3,   0,   0,   0,  17,   0, 
      0,   8,  18,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     17,   0,   0,   8,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  17,   0,   0,   8, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  17,   0, 
      0,   8, 130,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     54,   0,   0,   5, 114,  32, 
     16,   0,   5,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  17,   0,   0,   8, 
     18,  32,  16,   0,   7,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      4,   0,   0,   0,  17,   0, 
      0,   8,  34,  32,  16,   0, 
      7,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   5,   0,   0,   0, 
     17,   0,   0,   8,  66,  32, 
     16,   0,   7,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70, 142,  32,   0, 
      1,   0,   0,   0,   6,   0, 
      0,   0,  17,   0,   0,   8, 
    130,  32,  16,   0,   7,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      7,   0,   0,   0,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,  35,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,  12,   0, 
      0,   0,  26,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
