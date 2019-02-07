/*
================================================================================================

Description :   Vulkan format conversion from OpenGL.
Author      :   J.M.P. van Waveren
Date        :   07/17/2016
Language    :   C99
Copyright   :   Copyright (c) 2016 Oculus VR, LLC. All Rights reserved.


LICENSE
-------

Copyright (c) 2016 Oculus VR, LLC.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

================================================================================================
*/

#if !defined( VK_FORMAT_H )
#define VK_FORMAT_H

static inline VkFormat vkGetFormatFromOpenGLInternalFormat( const GLenum internalFormat )
{
    switch ( internalFormat )
    {
        //
        // 8 bits per component
        //
#ifdef GL_R8
        case GL_R8:                                             return VK_FORMAT_R8_UNORM;                  // 1-component, 8-bit unsigned normalized
        case GL_RG8:                                            return VK_FORMAT_R8G8_UNORM;                // 2-component, 8-bit unsigned normalized
#endif
        case GL_RGB8:                                           return VK_FORMAT_R8G8B8_UNORM;              // 3-component, 8-bit unsigned normalized
        case GL_RGBA8:                                          return VK_FORMAT_R8G8B8A8_UNORM;            // 4-component, 8-bit unsigned normalized

        case GL_R8_SNORM:                                       return VK_FORMAT_R8_SNORM;                  // 1-component, 8-bit signed normalized
        case GL_RG8_SNORM:                                      return VK_FORMAT_R8G8_SNORM;                // 2-component, 8-bit signed normalized
        case GL_RGB8_SNORM:                                     return VK_FORMAT_R8G8B8_SNORM;              // 3-component, 8-bit signed normalized
        case GL_RGBA8_SNORM:                                    return VK_FORMAT_R8G8B8A8_SNORM;            // 4-component, 8-bit signed normalized

        case GL_R8UI:                                           return VK_FORMAT_R8_UINT;                   // 1-component, 8-bit unsigned integer
        case GL_RG8UI:                                          return VK_FORMAT_R8G8_UINT;                 // 2-component, 8-bit unsigned integer
        case GL_RGB8UI:                                         return VK_FORMAT_R8G8B8_UINT;               // 3-component, 8-bit unsigned integer
        case GL_RGBA8UI:                                        return VK_FORMAT_R8G8B8A8_UINT;             // 4-component, 8-bit unsigned integer

        case GL_R8I:                                            return VK_FORMAT_R8_SINT;                   // 1-component, 8-bit signed integer
        case GL_RG8I:                                           return VK_FORMAT_R8G8_SINT;                 // 2-component, 8-bit signed integer
        case GL_RGB8I:                                          return VK_FORMAT_R8G8B8_SINT;               // 3-component, 8-bit signed integer
        case GL_RGBA8I:                                         return VK_FORMAT_R8G8B8A8_SINT;             // 4-component, 8-bit signed integer

        case GL_SR8_EXT:                                        return VK_FORMAT_R8_SRGB;                   // 1-component, 8-bit sRGB
#ifdef GL_SRG8_EXT
        case GL_SRG8_EXT:                                       return VK_FORMAT_R8G8_SRGB;                 // 2-component, 8-bit sRGB
#endif
#ifdef GL_SRGB8_NV
        case GL_SRGB8_NV:                                       return VK_FORMAT_R8G8B8_SRGB;               // 3-component, 8-bit sRGB
#endif
#ifdef GL_SRGB8_ALPHA8
        case GL_SRGB8_ALPHA8:                                   return VK_FORMAT_R8G8B8A8_SRGB;             // 4-component, 8-bit sRGB
#endif
        //
        // 16 bits per component
        //
#ifdef GL_R16_EXT
        case GL_R16_EXT:                                            return VK_FORMAT_R16_UNORM;                 // 1-component, 16-bit unsigned normalized
        case GL_RG16_EXT:                                           return VK_FORMAT_R16G16_UNORM;              // 2-component, 16-bit unsigned normalized
#endif
        case GL_RGB16_EXT:                                          return VK_FORMAT_R16G16B16_UNORM;           // 3-component, 16-bit unsigned normalized
        case GL_RGBA16_EXT:                                         return VK_FORMAT_R16G16B16A16_UNORM;        // 4-component, 16-bit unsigned normalized

#ifdef GL_R16_SNORM_EXT
        case GL_R16_SNORM_EXT:                                      return VK_FORMAT_R16_SNORM;                 // 1-component, 16-bit signed normalized
        case GL_RG16_SNORM_EXT:                                     return VK_FORMAT_R16G16_SNORM;              // 2-component, 16-bit signed normalized
        case GL_RGB16_SNORM_EXT:                                    return VK_FORMAT_R16G16B16_SNORM;           // 3-component, 16-bit signed normalized
        case GL_RGBA16_SNORM_EXT:                                   return VK_FORMAT_R16G16B16A16_SNORM;        // 4-component, 16-bit signed normalized
#endif

        case GL_R16UI:                                          return VK_FORMAT_R16_UINT;                  // 1-component, 16-bit unsigned integer
        case GL_RG16UI:                                         return VK_FORMAT_R16G16_UINT;               // 2-component, 16-bit unsigned integer
        case GL_RGB16UI:                                        return VK_FORMAT_R16G16B16_UINT;            // 3-component, 16-bit unsigned integer
        case GL_RGBA16UI:                                       return VK_FORMAT_R16G16B16A16_UINT;         // 4-component, 16-bit unsigned integer

        case GL_R16I:                                           return VK_FORMAT_R16_SINT;                  // 1-component, 16-bit signed integer
        case GL_RG16I:                                          return VK_FORMAT_R16G16_SINT;               // 2-component, 16-bit signed integer
        case GL_RGB16I:                                         return VK_FORMAT_R16G16B16_SINT;            // 3-component, 16-bit signed integer
        case GL_RGBA16I:                                        return VK_FORMAT_R16G16B16A16_SINT;         // 4-component, 16-bit signed integer

        case GL_R16F:                                           return VK_FORMAT_R16_SFLOAT;                // 1-component, 16-bit floating-point
        case GL_RG16F:                                          return VK_FORMAT_R16G16_SFLOAT;             // 2-component, 16-bit floating-point
        case GL_RGB16F:                                         return VK_FORMAT_R16G16B16_SFLOAT;          // 3-component, 16-bit floating-point
        case GL_RGBA16F:                                        return VK_FORMAT_R16G16B16A16_SFLOAT;       // 4-component, 16-bit floating-point

        //
        // 32 bits per component
        //
        case GL_R32UI:                                          return VK_FORMAT_R32_UINT;                  // 1-component, 32-bit unsigned integer
        case GL_RG32UI:                                         return VK_FORMAT_R32G32_UINT;               // 2-component, 32-bit unsigned integer
        case GL_RGB32UI:                                        return VK_FORMAT_R32G32B32_UINT;            // 3-component, 32-bit unsigned integer
        case GL_RGBA32UI:                                       return VK_FORMAT_R32G32B32A32_UINT;         // 4-component, 32-bit unsigned integer

        case GL_R32I:                                           return VK_FORMAT_R32_SINT;                  // 1-component, 32-bit signed integer
        case GL_RG32I:                                          return VK_FORMAT_R32G32_SINT;               // 2-component, 32-bit signed integer
        case GL_RGB32I:                                         return VK_FORMAT_R32G32B32_SINT;            // 3-component, 32-bit signed integer
        case GL_RGBA32I:                                        return VK_FORMAT_R32G32B32A32_SINT;         // 4-component, 32-bit signed integer

        case GL_R32F:                                           return VK_FORMAT_R32_SFLOAT;                // 1-component, 32-bit floating-point
        case GL_RG32F:                                          return VK_FORMAT_R32G32_SFLOAT;             // 2-component, 32-bit floating-point
        case GL_RGB32F:                                         return VK_FORMAT_R32G32B32_SFLOAT;          // 3-component, 32-bit floating-point
        case GL_RGBA32F:                                        return VK_FORMAT_R32G32B32A32_SFLOAT;       // 4-component, 32-bit floating-point

        //
        // Packed
        //
//        case GL_R3_G3_B2:                                       return VK_FORMAT_UNDEFINED;                 // 3-component 3:3:2,       unsigned normalized
//        case GL_RGB4:                                           return VK_FORMAT_UNDEFINED;                 // 3-component 4:4:4,       unsigned normalized
#ifdef GL_RGB5
        case GL_RGB5:                                           return VK_FORMAT_R5G5B5A1_UNORM_PACK16;     // 3-component 5:5:5,       unsigned normalized
#endif
        case GL_RGB565:                                         return VK_FORMAT_R5G6B5_UNORM_PACK16;       // 3-component 5:6:5,       unsigned normalized
        case GL_RGB10_EXT:                                          return VK_FORMAT_A2R10G10B10_UNORM_PACK32;  // 3-component 10:10:10,    unsigned normalized
        // case GL_RGB12:                                          return VK_FORMAT_UNDEFINED;                 // 3-component 12:12:12,    unsigned normalized
        // case GL_RGBA2:                                          return VK_FORMAT_UNDEFINED;                 // 4-component 2:2:2:2,     unsigned normalized
        case GL_RGBA4:                                          return VK_FORMAT_R4G4B4A4_UNORM_PACK16;     // 4-component 4:4:4:4,     unsigned normalized
//        case GL_RGBA12:                                         return VK_FORMAT_UNDEFINED;                 // 4-component 12:12:12:12, unsigned normalized
        case GL_RGB5_A1:                                        return VK_FORMAT_A1R5G5B5_UNORM_PACK16;     // 4-component 5:5:5:1,     unsigned normalized
        case GL_RGB10_A2:                                       return VK_FORMAT_A2R10G10B10_UNORM_PACK32;  // 4-component 10:10:10:2,  unsigned normalized
        case GL_RGB10_A2UI:                                     return VK_FORMAT_A2R10G10B10_UINT_PACK32;   // 4-component 10:10:10:2,  unsigned integer
        case GL_R11F_G11F_B10F:                                 return VK_FORMAT_B10G11R11_UFLOAT_PACK32;   // 3-component 11:11:10,    floating-point
        case GL_RGB9_E5:                                        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;    // 3-component/exp 9:9:9/5, floating-point

        //
        // S3TC/DXT/BC
        //

        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:                   return VK_FORMAT_BC1_RGB_UNORM_BLOCK;       // line through 3D space, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:                  return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;      // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:                  return VK_FORMAT_BC2_UNORM_BLOCK;           // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:                  return VK_FORMAT_BC3_UNORM_BLOCK;           // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:                  return VK_FORMAT_BC1_RGB_SRGB_BLOCK;        // line through 3D space, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;       // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:            return VK_FORMAT_BC2_SRGB_BLOCK;            // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:            return VK_FORMAT_BC3_SRGB_BLOCK;            // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB
#ifdef GL_COMPRESSED_LUMINANCE_LATC1_EXT
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:                 return VK_FORMAT_BC4_UNORM_BLOCK;           // line through 1D space, 4x4 blocks, unsigned normalized
#endif
#ifdef GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:           return VK_FORMAT_BC5_UNORM_BLOCK;           // two lines through 1D space, 4x4 blocks, unsigned normalized
#endif
#ifdef GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:          return VK_FORMAT_BC4_SNORM_BLOCK;           // line through 1D space, 4x4 blocks, signed normalized
#endif
#ifdef GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:    return VK_FORMAT_BC5_SNORM_BLOCK;           // two lines through 1D space, 4x4 blocks, signed normalized
#endif
        case GL_COMPRESSED_RED_RGTC1_EXT:                           return VK_FORMAT_BC4_UNORM_BLOCK;           // line through 1D space, 4x4 blocks, unsigned normalized
#ifdef GL_COMPRESSED_RG_RGTC2_EXT
        case GL_COMPRESSED_RG_RGTC2_EXT:                            return VK_FORMAT_BC5_UNORM_BLOCK;           // two lines through 1D space, 4x4 blocks, unsigned normalized
#endif
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:                    return VK_FORMAT_BC4_SNORM_BLOCK;           // line through 1D space, 4x4 blocks, signed normalized
#ifdef GL_COMPRESSED_SIGNED_RG_RGTC2_EXT
        case GL_COMPRESSED_SIGNED_RG_RGTC2_EXT:                     return VK_FORMAT_BC5_SNORM_BLOCK;           // two lines through 1D space, 4x4 blocks, signed normalized
#endif

#if defined(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT)
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;         // 3-component, 4x4 blocks, unsigned floating-point
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:               return VK_FORMAT_BC6H_SFLOAT_BLOCK;         // 3-component, 4x4 blocks, signed floating-point
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:                     return VK_FORMAT_BC7_UNORM_BLOCK;           // 4-component, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:               return VK_FORMAT_BC7_SRGB_BLOCK;            // 4-component, 4x4 blocks, sRGB
#elif defined(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT)
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;         // 3-component, 4x4 blocks, unsigned floating-point
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:               return VK_FORMAT_BC6H_SFLOAT_BLOCK;         // 3-component, 4x4 blocks, signed floating-point
        case GL_COMPRESSED_RGBA_BPTC_UNORM:                     return VK_FORMAT_BC7_UNORM_BLOCK;           // 4-component, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:               return VK_FORMAT_BC7_SRGB_BLOCK;            // 4-component, 4x4 blocks, sRGB
#endif
        //
        // ETC
        //
#ifdef GL_ETC1_RGB8_OES
        case GL_ETC1_RGB8_OES:                                  return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;   // 3-component ETC1, 4x4 blocks, unsigned normalized
#endif

        case GL_COMPRESSED_RGB8_ETC2:                           return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;   // 3-component ETC2, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:       return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA8_ETC2_EAC:                      return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK; // 4-component ETC2, 4x4 blocks, unsigned normalized

        case GL_COMPRESSED_SRGB8_ETC2:                          return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;    // 3-component ETC2, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:      return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;  // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:               return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;  // 4-component ETC2, 4x4 blocks, sRGB

        case GL_COMPRESSED_R11_EAC:                             return VK_FORMAT_EAC_R11_UNORM_BLOCK;       // 1-component ETC, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RG11_EAC:                            return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;    // 2-component ETC, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_SIGNED_R11_EAC:                      return VK_FORMAT_EAC_R11_SNORM_BLOCK;       // 1-component ETC, 4x4 blocks, signed normalized
        case GL_COMPRESSED_SIGNED_RG11_EAC:                     return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;    // 2-component ETC, 4x4 blocks, signed normalized

        //
        // ASTC
        //
        case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:                   return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;      // 4-component ASTC, 4x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:                   return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;      // 4-component ASTC, 5x4 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:                   return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;      // 4-component ASTC, 5x5 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:                   return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;      // 4-component ASTC, 6x5 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:                   return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;      // 4-component ASTC, 6x6 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:                   return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;      // 4-component ASTC, 8x5 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:                   return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;      // 4-component ASTC, 8x6 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:                   return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;      // 4-component ASTC, 8x8 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:                  return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;     // 4-component ASTC, 10x5 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:                  return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;     // 4-component ASTC, 10x6 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:                  return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;     // 4-component ASTC, 10x8 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:                 return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;    // 4-component ASTC, 10x10 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:                 return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;    // 4-component ASTC, 12x10 blocks, unsigned normalized
        case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:                 return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;    // 4-component ASTC, 12x12 blocks, unsigned normalized

        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:           return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;       // 4-component ASTC, 4x4 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:           return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;       // 4-component ASTC, 5x4 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:           return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;       // 4-component ASTC, 5x5 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:           return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;       // 4-component ASTC, 6x5 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:           return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;       // 4-component ASTC, 6x6 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:           return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;       // 4-component ASTC, 8x5 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:           return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;       // 4-component ASTC, 8x6 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:           return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;       // 4-component ASTC, 8x8 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:          return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;      // 4-component ASTC, 10x5 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:          return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;      // 4-component ASTC, 10x6 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:          return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;      // 4-component ASTC, 10x8 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:         return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;     // 4-component ASTC, 10x10 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:         return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;     // 4-component ASTC, 12x10 blocks, sRGB
        case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:         return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;     // 4-component ASTC, 12x12 blocks, sRGB

        //
        // Depth/stencil
        //
        case GL_DEPTH_COMPONENT16:                              return VK_FORMAT_D16_UNORM;
        case GL_DEPTH_COMPONENT24:                              return VK_FORMAT_X8_D24_UNORM_PACK32;
//        case GL_DEPTH_COMPONENT32:                              return VK_FORMAT_UNDEFINED;
        case GL_DEPTH_COMPONENT32F:                             return VK_FORMAT_D32_SFLOAT;
#ifdef GL_DEPTH_COMPONENT32F_NV
        case GL_DEPTH_COMPONENT32F_NV:                          return VK_FORMAT_D32_SFLOAT;
#endif
//        case GL_STENCIL_INDEX1:                                 return VK_FORMAT_UNDEFINED;
//        case GL_STENCIL_INDEX4:                                 return VK_FORMAT_UNDEFINED;
        case GL_STENCIL_INDEX8:                                 return VK_FORMAT_S8_UINT;
//        case GL_STENCIL_INDEX16:                                return VK_FORMAT_UNDEFINED;
        case GL_DEPTH24_STENCIL8:                               return VK_FORMAT_D24_UNORM_S8_UINT;
        case GL_DEPTH32F_STENCIL8:                              return VK_FORMAT_D32_SFLOAT_S8_UINT;
#ifdef GL_DEPTH32F_STENCIL8_NV
        case GL_DEPTH32F_STENCIL8_NV:                           return VK_FORMAT_D32_SFLOAT_S8_UINT;
#endif
        default:                                                return VK_FORMAT_UNDEFINED;
    }
}

#endif // !VK_FORMAT_H
