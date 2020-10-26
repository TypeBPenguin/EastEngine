Texture2D Tex2DTable[] : register(t0, space0);
Texture2DArray Tex2DArrayTable[] : register(t0, space1);
TextureCube TexCubeTable[] : register(t0, space2);
Texture3D Tex3DTable[] : register(t0, space3);
Texture2DMS<float4> Tex2DMSTable[] : register(t0, space4);
ByteAddressBuffer RawBufferTable[] : register(t0, space5);
Buffer<uint> BufferUintTable[] : register(t0, space6);
Texture1D Tex1DTable[] : register(t0, space7);
Texture2D<float> Tex2DFloatTable[] : register(t0, space8);

RWTexture2D<float4> Uav2DTable[] : register(u0, space0);

SamplerState SamplerTable[]: register(s0, space0);