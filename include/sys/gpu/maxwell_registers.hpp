/*
    Nozomi is an experimental HLE Switch emulator.
    Copyright (C) 2023  noumidev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "types.hpp"

namespace sys::gpu::maxwell {

constexpr u32 NUM_SCISSOR_AREAS = 16;
constexpr u32 NUM_COLOR_TARGETS = 8;
constexpr u32 NUM_VIEWPORTS = 16;
constexpr u32 NUM_WINDOWS = 8;
constexpr u32 NUM_POLY_STIPPLE_PATTERNS = 32;
constexpr u32 NUM_VERTEX_STREAMS = 4; // ??
constexpr u32 NUM_BIND_GROUPS = 5;
constexpr u32 NUM_MME_REGISTERS = 128;

namespace Register {
    enum : u32 {
        SetObject,
        LoadMmeInstructionRamPointer = 0x045,
        LoadMmeInstructionRam,
        LoadMmeStartAddressRamPointer,
        LoadMmeStartAddressRam,
        SetAliasedLineWidthEnable = 0x083,
        SetL2CacheControlForRopPrefetchReadRequests = 0x086,
        InvalidateShaderCaches,
        IncrementSyncPoint = 0x0B2,
        SetPrimCircularBufferThrottle = 0x0B4,
        SetPsOutputSampleMaskUsage = 0x0C0,
        SetL1Configuration = 0x0C2,
        SetRenderEnableControl,
        SetTessellationParameters = 0x0C8,
        SetTessellationLodU0OrDensity,
        SetTessellationLodV0OrDetail,
        SetTessellationLodU1OrW0,
        SetTessellationLodV1,
        SetTgLodInteriorU,
        SetTgLodInteriorV,
        SetSubtilingPerfKnobA = 0x0D8,
        SetSubtilingPerfKnobB,
        SetSubtilingPerfKnobC,
        SetRasterEnable = 0x0DF,
        SetAlphaFraction = 0x1D3,
        SetHybridAntiAliasControl = 0x1D5,
        SetShaderLocalMemoryWindow = 0x1DF,
        SetShaderLocalMemoryA = 0x1E4,
        SetShaderLocalMemoryB,
        SetShaderLocalMemoryC,
        SetShaderLocalMemoryD,
        SetShaderLocalMemoryE,
        SetColorTargetA = 0x200,
        SetColorTargetB,
        SetColorTargetWidth,
        SetColorTargetHeight,
        SetColorTargetFormat,
        SetColorTargetMemory,
        SetColorTargetThirdDimension,
        SetColorTargetArrayPitch,
        SetColorTargetLayer,
        SetColorTargetMark,
        SetViewportScaleX = 0x280,
        SetViewportScaleY,
        SetViewportScaleZ,
        SetViewportOffsetX,
        SetViewportOffsetY,
        SetViewportOffsetZ,
        SetViewportCoordinateSwizzle,
        SetViewportIncreaseSnapGridPrecision,
        SetViewportClipHorizontal = 0x300,
        SetViewportClipVertical,
        SetViewportClipMinZ,
        SetViewportClipMaxZ,
        SetWindowClipHorizonzal = 0x340,
        SetWindowClipVertical,
        SetApiVisibleCallLimit = 0x359,
        SetVertexArrayStart = 0x35D,
        DrawVertexArray,
        SetViewportZClip = 0x35F,
        SetColorClearValue,
        InvalidateShaderCachesNoWfi = 0x369,
        SetPolySmooth = 0x36D,
        SetPolyOffsetPoint = 0x370,
        SetPolyOffsetLine,
        SetPolyOffsetFill,
        SetPatch,
        SetSmTimeoutInterval = 0x379,
        SetDaPrimitiveRestartVertexArray,
        SetWindowOffsetX = 0x37E,
        SetWindowOffsetY,
        SetScissorEnable,
        SetScissorHorizontal,
        SetScissorVertical,
        SetVertexStreamSubstituteA = 0x3E1,
        SetVertexStreamSubstituteB,
        SetSingleCtWriteControl = 0x3E4,
        SetCtMrtEnable = 0x3EB,
        SetBlendOptControl = 0x3F7,
        SetZtA,
        SetZtB,
        SetZtFormat,
        SetZtBlockSize,
        SetZtArrayPitch,
        SetSurfaceClipHorizontal,
        SetSurfaceClipVertical,
        SetReduceColorThresholdsUnorm8 = 0x433,
        SetReduceColorThresholdsUnorm10 = 0x438,
        SetReduceColorThresholdsUnorm16,
        SetReduceColorThresholdsFp16 = 0x43B,
        SetReduceColorThresholdsSrgb8,
        SetClearSurfaceControl = 0x43E,
        SetL2CacheControlForRopNoninterlockedReadRequests,
        SetFillViaTriangle = 0x44F,
        SetBlendPerFormatEnable,
        FlushPendingWrites,
        RasterEnable,
        SetVertexAttributeA = 0x458,
        SetVertexAttributeB = 0x468,
        SetAntiAliasSamplePositions = 0x478,
        SetOffsetRenderTargetIndex = 0x47C,
        SetCtSelect = 0x487,
        SetZtSizeA = 0x48A,
        SetZtSizeB,
        SetZtSizeC,
        SetSamplerBinding,
        InvalidateTextureDataCacheNoWfi = 0x4A2,
        SetL2CacheControlForRopInterlockedReadRequests = 0x4A4,
        SetDepthTest = 0x4B3,
        SetShadeMode = 0x4B5,
        SetL2CacheControlForRopNoninterlockedWriteRequests,
        SetL2CacheControlForRopInterlockedWriteRequests,
        SetBlendStatePerTarget = 0x4B9,
        SetAlphaTest = 0x4BB,
        InvalidateSamplerCache = 0x4CC,
        InvalidateTextureHeaderCache,
        InvalidateTextureDataCache,
        SetBlendSeperateForAlpha,
        SetSingleRopControl = 0x4D7,
        SetStencilTest = 0x4E0,
        SetStencilOpFail,
        SetStencilOpZfail,
        SetStencilOpZpass,
        SetStencilFunc,
        SetStencilFuncRef,
        SetStencilFuncMask,
        SetStencilMask,
        SetPsSaturate = 0x4EA,
        SetWindowOrigin,
        SetAliasedLineWidthFloat = 0x4ED,
        InvalidateSamplerCacheNoWfi = 0x509,
        InvalidateTextureHeaderCacheNoWfi,
        SetPointSize = 0x546,
        SetZcullStats,
        SetPointSprite,
        SetAntiAliasEnable = 0x54D,
        SetZtSelect,
        SetAntiAliasAlphaControl,
        SetRenderEnableA = 0x554,
        SetRenderEnableB,
        SetRenderEnableC,
        SetTexSamplerPoolA,
        SetTexSamplerPoolB,
        SetTexSamplerPoolC,
        SetAntiAliasedLine = 0x55C,
        SetTexHeaderPoolA,
        SetTexHeaderPoolB,
        SetTexHeaderPoolC,
        SetActiveZcullRegion = 0x564,
        SetCsaa = 0x56D,
        SetRtLayer = 0x573,
        SetAntiAlias,
        SetEdgeFlag = 0x579,
        SetPointSpriteSelect = 0x581,
        SetProgramRegionA,
        SetProgramRegionB,
        SetAttributeDefault,
        End,
        Begin,
        SetDaOutput = 0x593,
        SetAntiAliasedPoint = 0x596,
        SetPointCenterMode,
        SetLineStipple = 0x59B,
        SetProvokingVertex = 0x5A1,
        SetTwoSidedLight,
        SetPolygonStipple,
        CheckSphVersion = 0x5AA,
        SetPolygonStipplePattern = 0x5C0,
        CheckAamVersion = 0x5E5,
        SetZtLayer = 0x5E7,
        SetAttributePointSize = 0x644,
        OglSetCull = 0x646,
        OglSetFrontFace,
        OglSetCullFace,
        SetViewportPixel = 0x649,
        SetViewportScaleOffset = 0x64B,
        SetViewportClipControl = 0x64F,
        SetWindowClipEnable = 0x653,
        SetWindowClipType,
        SetZcull = 0x65A,
        SetZcullBounds,
        SetClipIdTest = 0x65F,
        SetDepthBoundsTest = 0x66F,
        SetBlendFloatOption,
        SetLogicOp,
        SetLogicOpFunc,
        SetZCompression,
        ClearSurface,
        SetColorCompression = 0x678,
        SetCtWrite = 0x680,
        SetReportSemaphoreA = 0x6C0,
        SetReportSemaphoreB,
        SetReportSemaphoreC,
        SetReportSemaphoreD,
        SetVertexStreamAFormat = 0x700,
        SetVertexStreamALocationA,
        SetVertexStreamALocationB,
        SetVertexStreamAFrequency,
        SetVertexStreamLimitAA = 0x7C0,
        SetVertexStreamLimitAB,
        SetPipelineShader = 0x800,
        SetPipelineProgram,
        SetPipelineReservedA,
        SetPipelineRegisterCount,
        SetPipelineBinding,
        SetPipelineReservedB,
        SetPipelineReservedC,
        SetPipelineReservedD,
        SetPipelineReservedE,
        SetTesselationProgram = 0x820,
        SetTesselationProgramRegion,
        SetFragmentProgram = 0x850,
        SetFragmentProgramRegion,
        SetFragmentProgramRegisterCount = 0x853,
        SetFragmentProgramBindGroup,
        SetConstantBufferSelectorA = 0x8E0,
        SetConstantBufferSelectorB,
        SetConstantBufferSelectorC,
        LoadConstantBufferOffset,
        LoadConstantBuffer,
        BindGroupReservedA = 0x900,
        BindGroupReservedB,
        BindGroupReservedC,
        BindGroupReservedD,
        BindGroupConstantBuffer,
        SetColorClamp = 0x980,
        SetBindlessTexture = 0x982,
        CallMmeMacro = 0xE00,
        CallMmeData,
    };
}

}
