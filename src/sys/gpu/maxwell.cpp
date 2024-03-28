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

#include "maxwell.hpp"

#include <array>
#include <cstdlib>
#include <ios>

#include <plog/Log.h>

#include "maxwell_registers.hpp"

namespace sys::gpu::maxwell {

constexpr bool ENABLE_WRITE_LOG = true;

constexpr u32 NUM_REGS = 0x1000;
constexpr u32 NUM_DMA_REGS = 0x800;

std::array<u32, NUM_REGS> regs;
std::array<u32, NUM_DMA_REGS> dmaRegs;

const char *getRenderEnableCMsg() {
    switch (regs[Register::SetRenderEnableC]) {
        case 1:
            return "true";
        default:
            PLOG_FATAL << "Invalid enum";

            exit(0);
    }
}

void write(u32 addr, u32 data) {
    if (addr > NUM_REGS) {
        PLOG_FATAL << "Invalid register address " << std::hex << addr;

        exit(0);
    }

    regs[addr] = data;

    switch (addr) {
        case Register::LoadMmeInstructionRam:
            // TODO: figure out what this RAM is for
            regs[Register::LoadMmeInstructionRamPointer]++;
            break;
        case Register::LoadMmeStartAddressRam:
            // TODO: does this autoincrement the pointer?
            regs[Register::LoadMmeStartAddressRamPointer]++;
            break;
        default:
            break;
    }

    if constexpr (ENABLE_WRITE_LOG) {
        switch (addr) {
            case Register::SetObject:
                PLOG_INFO << "SetObject (data = " << std::hex << data << ")";
                break;
            case Register::LoadMmeInstructionRamPointer:
                PLOG_INFO << "LoadMmeInstructionRamPointer (data = " << std::hex << data << ")";
                break;
            case Register::LoadMmeInstructionRam:
                PLOG_INFO << "LoadMmeInstructionRam (data = " << std::hex << data << ")";
                break;
            case Register::LoadMmeStartAddressRamPointer:
                PLOG_INFO << "LoadMmeStartAddressRamPointer (data = " << std::hex << data << ")";
                break;
            case Register::LoadMmeStartAddressRam:
                PLOG_INFO << "LoadMmeStartAddressRam (data = " << std::hex << data << ")";
                break;
            case Register::SetAliasedLineWidthEnable:
                PLOG_INFO << "SetAliasedLineWidthEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetL2CacheControlForRopPrefetchReadRequests:
                PLOG_INFO << "SetL2CacheControlForRopPrefetchReadRequests (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateShaderCaches:
                PLOG_INFO << "InvalidateShaderCaches (data = " << std::hex << data << ")";
                break;
            case Register::IncrementSyncPoint:
                PLOG_INFO << "IncrementSyncPoint (data = " << std::hex << data << ")";
                break;
            case Register::SetPrimCircularBufferThrottle:
                PLOG_INFO << "SetPrimCircularBufferThrottle (data = " << std::hex << data << ")";
                break;
            case Register::SetPsOutputSampleMaskUsage:
                PLOG_INFO << "SetPsOutputSampleMaskUsage (data = " << std::hex << data << ")";
                break;
            case Register::SetL1Configuration:
                PLOG_INFO << "SetL1Configuration (data = " << std::hex << data << ")";
                break;
            case Register::SetRenderEnableControl:
                PLOG_INFO << "SetRenderEnableControl (data = " << std::hex << data << ")";
                break;
            case Register::SetTessellationParameters:
                PLOG_INFO << "SetTessellationParameters (data = " << std::hex << data << ")";
                break;
            case Register::SetTessellationLodU0OrDensity:
                PLOG_INFO << "SetTessellationLodU0OrDensity (data = " << std::hex << data << ")";
                break;
            case Register::SetTessellationLodV0OrDetail:
                PLOG_INFO << "SetTessellationLodV0OrDetail (data = " << std::hex << data << ")";
                break;
            case Register::SetTessellationLodU1OrW0:
                PLOG_INFO << "SetTessellationLodU1OrW0 (data = " << std::hex << data << ")";
                break;
            case Register::SetTessellationLodV1:
                PLOG_INFO << "SetTessellationLodV1 (data = " << std::hex << data << ")";
                break;
            case Register::SetTgLodInteriorU:
                PLOG_INFO << "SetTgLodInteriorU (data = " << std::hex << data << ")";
                break;
            case Register::SetTgLodInteriorV:
                PLOG_INFO << "SetTgLodInteriorV (data = " << std::hex << data << ")";
                break;
            case Register::SetSubtilingPerfKnobA:
                PLOG_INFO << "SetSubtilingPerfKnobA (data = " << std::hex << data << ")";
                break;
            case Register::SetSubtilingPerfKnobB:
                PLOG_INFO << "SetSubtilingPerfKnobB (data = " << std::hex << data << ")";
                break;
            case Register::SetSubtilingPerfKnobC:
                PLOG_INFO << "SetSubtilingPerfKnobC (data = " << std::hex << data << ")";
                break;
            case Register::SetRasterEnable:
                PLOG_INFO << "SetRasterEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetAlphaFraction:
                PLOG_INFO << "SetAlphaFraction (data = " << std::hex << data << ")";
                break;
            case Register::SetHybridAntiAliasControl:
                PLOG_INFO << "SetHybridAntiAliasControl (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryWindow:
                PLOG_INFO << "SetShaderLocalMemoryWindow (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryA:
                PLOG_INFO << "SetShaderLocalMemoryA (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryB:
                PLOG_INFO << "SetShaderLocalMemoryB (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryC:
                PLOG_INFO << "SetShaderLocalMemoryC (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryD:
                PLOG_INFO << "SetShaderLocalMemoryD (data = " << std::hex << data << ")";
                break;
            case Register::SetShaderLocalMemoryE:
                PLOG_INFO << "SetShaderLocalMemoryE (data = " << std::hex << data << ")";
                break;
            case Register::SetApiVisibleCallLimit:
                PLOG_INFO << "SetApiVisibleCallLimit (data = " << std::hex << data << ")";
                break;
            case Register::SetVertexArrayStart:
                PLOG_INFO << "SetVertexArrayStart (data = " << std::hex << data << ")";
                break;
            case Register::DrawVertexArray:
                PLOG_INFO << "DrawVertexArray (data = " << std::hex << data << ")";
                break;
            case Register::SetViewportZClip:
                PLOG_INFO << "SetViewportZClip (data = " << std::hex << data << ")";
                break;
            case Register::SetColorClearValue:
            case Register::SetColorClearValue + 1:
            case Register::SetColorClearValue + 2:
            case Register::SetColorClearValue + 3:
                PLOG_INFO << "SetColorClearValue" << addr - Register::SetColorClearValue << " (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateShaderCachesNoWfi:
                PLOG_INFO << "InvalidateShaderCachesNoWfi (data = " << std::hex << data << ")";
                break;
            case Register::SetPolySmooth:
                PLOG_INFO << "SetPolySmooth (data = " << std::hex << data << ")";
                break;
            case Register::SetPolyOffsetPoint:
                PLOG_INFO << "SetPolyOffsetPoint (data = " << std::hex << data << ")";
                break;
            case Register::SetPolyOffsetLine:
                PLOG_INFO << "SetPolyOffsetLine (data = " << std::hex << data << ")";
                break;
            case Register::SetPolyOffsetFill:
                PLOG_INFO << "SetPolyOffsetFill (data = " << std::hex << data << ")";
                break;
            case Register::SetPatch:
                PLOG_INFO << "SetPatch (data = " << std::hex << data << ")";
                break;
            case Register::SetSmTimeoutInterval:
                PLOG_INFO << "SetSmTimeoutInterval (data = " << std::hex << data << ")";
                break;
            case Register::SetDaPrimitiveRestartVertexArray:
                PLOG_INFO << "SetDaPrimitiveRestartVertexArray (enable = " << std::hex << data << ")";
                break;
            case Register::SetWindowOffsetX:
                PLOG_INFO << "SetWindowOffsetX (data = " << std::hex << data << ")";
                break;
            case Register::SetWindowOffsetY:
                PLOG_INFO << "SetWindowOffsetY (data = " << std::hex << data << ")";
                break;
            case Register::SetVertexStreamSubstituteA:
                PLOG_INFO << "SetVertexStreamSubstituteA (data = " << std::hex << data << ")";
                break;
            case Register::SetVertexStreamSubstituteB:
                PLOG_INFO << "SetVertexStreamSubstituteB (data = " << std::hex << data << ")";
                break;
            case Register::SetSingleCtWriteControl:
                PLOG_INFO << "SetSingleCtWriteControl (data = " << std::hex << data << ")";
                break;
            case Register::SetCtMrtEnable:
                PLOG_INFO << "SetCtMrtEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetBlendOptControl:
                PLOG_INFO << "SetBlendOptControl (data = " << std::hex << data << ")";
                break;
            case Register::SetZtA:
                PLOG_INFO << "SetZtA (data = " << std::hex << data << ")";
                break;
            case Register::SetZtB:
                PLOG_INFO << "SetZtB (data = " << std::hex << data << ")";
                break;
            case Register::SetZtFormat:
                PLOG_INFO << "SetZtFormat (data = " << std::hex << data << ")";
                break;
            case Register::SetZtBlockSize:
                PLOG_INFO << "SetZtBlockSize (data = " << std::hex << data << ")";
                break;
            case Register::SetZtArrayPitch:
                PLOG_INFO << "SetZtArrayPitch (data = " << std::hex << data << ")";
                break;
            case Register::SetSurfaceClipHorizontal:
                PLOG_INFO << "SetSurfaceClipHorizontal (data = " << std::hex << data << ")";
                break;
            case Register::SetSurfaceClipVertical:
                PLOG_INFO << "SetSurfaceClipVertical (data = " << std::hex << data << ")";
                break;
            case Register::SetReduceColorThresholdsUnorm8:
                PLOG_INFO << "SetReduceColorThresholdsUnorm8 (data = " << std::hex << data << ")";
                break;
            case Register::SetReduceColorThresholdsUnorm10:
                PLOG_INFO << "SetReduceColorThresholdsUnorm10 (data = " << std::hex << data << ")";
                break;
            case Register::SetReduceColorThresholdsUnorm16:
                PLOG_INFO << "SetReduceColorThresholdsUnorm16 (data = " << std::hex << data << ")";
                break;
            case Register::SetReduceColorThresholdsFp16:
                PLOG_INFO << "SetReduceColorThresholdsFp16 (data = " << std::hex << data << ")";
                break;
            case Register::SetReduceColorThresholdsSrgb8:
                PLOG_INFO << "SetReduceColorThresholdsSrgb8 (data = " << std::hex << data << ")";
                break;
            case Register::SetClearSurfaceControl:
                PLOG_INFO << "SetClearSurfaceControl (data = " << std::hex << data << ")";
                break;
            case Register::SetL2CacheControlForRopNoninterlockedReadRequests:
                PLOG_INFO << "SetL2CacheControlForRopNoninterlockedReadRequests (data = " << std::hex << data << ")";
                break;
            case Register::SetFillViaTriangle:
                PLOG_INFO << "SetFillViaTriangle (data = " << std::hex << data << ")";
                break;
            case Register::SetBlendPerFormatEnable:
                PLOG_INFO << "SetBlendPerFormatEnable (data = " << std::hex << data << ")";
                break;
            case Register::FlushPendingWrites:
                PLOG_INFO << "FlushPendingWrites (data = " << std::hex << data << ")";
                break;
            case Register::RasterEnable:
                PLOG_INFO << "RasterEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetVertexAttributeA:
            case Register::SetVertexAttributeA + 1:
            case Register::SetVertexAttributeA + 2:
            case Register::SetVertexAttributeA + 3:
                PLOG_INFO << "SetVertexAttributeA" << addr - Register::SetVertexAttributeA << " (data = " << std::hex << data << ")";
                break;
            case Register::SetVertexAttributeB:
            case Register::SetVertexAttributeB + 1:
            case Register::SetVertexAttributeB + 2:
            case Register::SetVertexAttributeB + 3:
                PLOG_INFO << "SetVertexAttributeB" << addr - Register::SetVertexAttributeB << " (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasSamplePositions:
            case Register::SetAntiAliasSamplePositions + 1:
            case Register::SetAntiAliasSamplePositions + 2:
            case Register::SetAntiAliasSamplePositions + 3:
                PLOG_INFO << "SetAntiAliasSamplePositions" << addr - Register::SetAntiAliasSamplePositions << " (data = " << std::hex << data << ")";
                break;
            case Register::SetOffsetRenderTargetIndex:
                PLOG_INFO << "SetOffsetRenderTargetIndex (data = " << std::hex << data << ")";
                break;
            case Register::SetCtSelect:
                PLOG_INFO << "SetCtSelect (data = " << std::hex << data << ")";
                break;
            case Register::SetZtSizeA:
                PLOG_INFO << "SetZtSizeA (data = " << std::hex << data << ")";
                break;
            case Register::SetZtSizeB:
                PLOG_INFO << "SetZtSizeB (data = " << std::hex << data << ")";
                break;
            case Register::SetZtSizeC:
                PLOG_INFO << "SetZtSizeC (data = " << std::hex << data << ")";
                break;
            case Register::SetSamplerBinding:
                PLOG_INFO << "SetSamplerBinding (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateTextureDataCacheNoWfi:
                PLOG_INFO << "InvalidateTextureDataCacheNoWfi (data = " << std::hex << data << ")";
                break;
            case Register::SetL2CacheControlForRopInterlockedReadRequests:
                PLOG_INFO << "SetL2CacheControlForRopInterlockedReadRequests (data = " << std::hex << data << ")";
                break;
            case Register::SetDepthTest:
                PLOG_INFO << "SetDepthTest (data = " << std::hex << data << ")";
                break;
            case Register::SetShadeMode:
                PLOG_INFO << "SetShadeMode (data = " << std::hex << data << ")";
                break;
            case Register::SetL2CacheControlForRopNoninterlockedWriteRequests:
                PLOG_INFO << "SetL2CacheControlForRopNoninterlockedWriteRequests (data = " << std::hex << data << ")";
                break;
            case Register::SetL2CacheControlForRopInterlockedWriteRequests:
                PLOG_INFO << "SetL2CacheControlForRopInterlockedWriteRequests (data = " << std::hex << data << ")";
                break;
            case Register::SetBlendStatePerTarget:
                PLOG_INFO << "SetBlendStatePerTarget (data = " << std::hex << data << ")";
                break;
            case Register::SetAlphaTest:
                PLOG_INFO << "SetAlphaTest (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateSamplerCache:
                PLOG_INFO << "InvalidateSamplerCache (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateTextureHeaderCache:
                PLOG_INFO << "InvalidateTextureHeaderCache (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateTextureDataCache:
                PLOG_INFO << "InvalidateTextureDataCache (data = " << std::hex << data << ")";
                break;
            case Register::SetBlendSeperateForAlpha:
                PLOG_INFO << "SetBlendSeperateForAlpha (data = " << std::hex << data << ")";
                break;
            case Register::SetSingleRopControl:
                PLOG_INFO << "SetSingleRopControl (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilTest:
                PLOG_INFO << "SetStencilTest (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilOpZfail:
                PLOG_INFO << "SetStencilOpZfail (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilOpZpass:
                PLOG_INFO << "SetStencilOpZpass (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilFunc:
                PLOG_INFO << "SetStencilFunc (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilFuncRef:
                PLOG_INFO << "SetStencilFuncRef (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilFuncMask:
                PLOG_INFO << "SetStencilFuncMask (data = " << std::hex << data << ")";
                break;
            case Register::SetStencilMask:
                PLOG_INFO << "SetStencilMask (data = " << std::hex << data << ")";
                break;
            case Register::SetPsSaturate:
                PLOG_INFO << "SetPsSaturate (data = " << std::hex << data << ")";
                break;
            case Register::SetWindowOrigin:
                PLOG_INFO << "SetWindowOrigin (data = " << std::hex << data << ")";
                break;
            case Register::SetAliasedLineWidthFloat:
                PLOG_INFO << "SetAliasedLineWidthFloat (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateSamplerCacheNoWfi:
                PLOG_INFO << "InvalidateSamplerCacheNoWfi (data = " << std::hex << data << ")";
                break;
            case Register::InvalidateTextureHeaderCacheNoWfi:
                PLOG_INFO << "InvalidateTextureHeaderCacheNoWfi (data = " << std::hex << data << ")";
                break;
            case Register::SetPointSize:
                PLOG_INFO << "SetPointSize (data = " << std::hex << data << ")";
                break;
            case Register::SetZcullStats:
                PLOG_INFO << "SetZcullStats (data = " << std::hex << data << ")";
                break;
            case Register::SetPointSprite:
                PLOG_INFO << "SetPointSprite (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasEnable:
                PLOG_INFO << "SetAntiAliasEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetZtSelect:
                PLOG_INFO << "SetZtSelect (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasAlphaControl:
                PLOG_INFO << "SetAntiAliasAlphaControl (data = " << std::hex << data << ")";
                break;
            case Register::SetRenderEnableC:
                PLOG_INFO << "SetRenderEnableC (render enable = " << getRenderEnableCMsg() << ")";
                break;
            case Register::SetTexSamplerPoolA:
                PLOG_INFO << "SetTexSamplerPoolA (data = " << std::hex << data << ")";
                break;
            case Register::SetTexSamplerPoolB:
                PLOG_INFO << "SetTexSamplerPoolB (data = " << std::hex << data << ")";
                break;
            case Register::SetTexSamplerPoolC:
                PLOG_INFO << "SetTexSamplerPoolC (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasedLine:
                PLOG_INFO << "SetAntiAliasedLine (data = " << std::hex << data << ")";
                break;
            case Register::SetTexHeaderPoolA:
                PLOG_INFO << "SetTexHeaderPoolA (data = " << std::hex << data << ")";
                break;
            case Register::SetTexHeaderPoolB:
                PLOG_INFO << "SetTexHeaderPoolB (data = " << std::hex << data << ")";
                break;
            case Register::SetTexHeaderPoolC:
                PLOG_INFO << "SetTexHeaderPoolC (data = " << std::hex << data << ")";
                break;
            case Register::SetActiveZcullRegion:
                PLOG_INFO << "SetActiveZcullRegion (data = " << std::hex << data << ")";
                break;
            case Register::SetCsaa:
                PLOG_INFO << "SetCsaa (data = " << std::hex << data << ")";
                break;
            case Register::SetRtLayer:
                PLOG_INFO << "SetRtLayer (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAlias:
                PLOG_INFO << "SetAntiAlias (data = " << std::hex << data << ")";
                break;
            case Register::SetEdgeFlag:
                PLOG_INFO << "SetEdgeFlag (data = " << std::hex << data << ")";
                break;
            case Register::SetPointSpriteSelect:
                PLOG_INFO << "SetPointSpriteSelect (data = " << std::hex << data << ")";
                break;
            case Register::SetProgramRegionA:
                PLOG_INFO << "SetProgramRegionA (data = " << std::hex << data << ")";
                break;
            case Register::SetProgramRegionB:
                PLOG_INFO << "SetProgramRegionB (data = " << std::hex << data << ")";
                break;
            case Register::SetAttributeDefault:
                PLOG_INFO << "SetAttributeDefault (data = " << std::hex << data << ")";
                break;
            case Register::End:
                PLOG_INFO << "End (data = " << std::hex << data << ")";
                break;
            case Register::Begin:
                PLOG_INFO << "Begin (data = " << std::hex << data << ")";
                break;
            case Register::SetDaOutput:
                PLOG_INFO << "SetDaOutput (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasedPoint:
                PLOG_INFO << "SetAntiAliasedPoint (data = " << std::hex << data << ")";
                break;
            case Register::SetPointCenterMode:
                PLOG_INFO << "SetPointCenterMode (data = " << std::hex << data << ")";
                break;
            case Register::SetLineStipple:
                PLOG_INFO << "SetLineStipple (data = " << std::hex << data << ")";
                break;
            case Register::SetProvokingVertex:
                PLOG_INFO << "SetProvokingVertex (data = " << std::hex << data << ")";
                break;
            case Register::SetTwoSidedLight:
                PLOG_INFO << "SetTwoSidedLight (data = " << std::hex << data << ")";
                break;
            case Register::SetPolygonStipple:
                PLOG_INFO << "SetPolygonStipple (data = " << std::hex << data << ")";
                break;
            case Register::CheckSphVersion:
                PLOG_INFO << "CheckSphVersion (data = " << std::hex << data << ")";
                break;
            case Register::CheckAamVersion:
                PLOG_INFO << "CheckAamVersion (data = " << std::hex << data << ")";
                break;
            case Register::SetZtLayer:
                PLOG_INFO << "SetZtLayer (data = " << std::hex << data << ")";
                break;
            case Register::SetAttributePointSize:
                PLOG_INFO << "SetAttributePointSize (data = " << std::hex << data << ")";
                break;
            case Register::OglSetCull:
                PLOG_INFO << "OglSetCull (data = " << std::hex << data << ")";
                break;
            case Register::OglSetFrontFace:
                PLOG_INFO << "OglSetFrontFace (data = " << std::hex << data << ")";
                break;
            case Register::OglSetCullFace:
                PLOG_INFO << "OglSetCullFace (data = " << std::hex << data << ")";
                break;
            case Register::SetViewportPixel:
                PLOG_INFO << "SetViewportPixel (data = " << std::hex << data << ")";
                break;
            case Register::SetViewportScaleOffset:
                PLOG_INFO << "SetViewportScaleOffset (data = " << std::hex << data << ")";
                break;
            case Register::SetViewportClipControl:
                PLOG_INFO << "SetViewportClipControl (data = " << std::hex << data << ")";
                break;
            case Register::SetWindowClipEnable:
                PLOG_INFO << "SetWindowClipEnable (data = " << std::hex << data << ")";
                break;
            case Register::SetWindowClipType:
                PLOG_INFO << "SetWindowClipType (data = " << std::hex << data << ")";
                break;
            case Register::SetZcull:
                PLOG_INFO << "SetZcull (data = " << std::hex << data << ")";
                break;
            case Register::SetZcullBounds:
                PLOG_INFO << "SetZcullBounds (data = " << std::hex << data << ")";
                break;
            case Register::SetClipIdTest:
                PLOG_INFO << "SetClipIdTest (data = " << std::hex << data << ")";
                break;
            case Register::SetDepthBoundsTest:
                PLOG_INFO << "SetDepthBoundsTest (data = " << std::hex << data << ")";
                break;
            case Register::SetBlendFloatOption:
                PLOG_INFO << "SetBlendFloatOption (data = " << std::hex << data << ")";
                break;
            case Register::SetLogicOp:
                PLOG_INFO << "SetLogicOp (data = " << std::hex << data << ")";
                break;
            case Register::SetLogicOpFunc:
                PLOG_INFO << "SetLogicOpFunc (data = " << std::hex << data << ")";
                break;
            case Register::SetZCompression:
                PLOG_INFO << "SetZCompression (data = " << std::hex << data << ")";
                break;
            case Register::ClearSurface:
                PLOG_INFO << "ClearSurface (data = " << std::hex << data << ")";
                break;
            case Register::SetReportSemaphoreA:
                PLOG_INFO << "SetReportSemaphoreA (data = " << std::hex << data << ")";
                break;
            case Register::SetReportSemaphoreB:
                PLOG_INFO << "SetReportSemaphoreB (data = " << std::hex << data << ")";
                break;
            case Register::SetReportSemaphoreC:
                PLOG_INFO << "SetReportSemaphoreC (data = " << std::hex << data << ")";
                break;
            case Register::SetReportSemaphoreD:
                PLOG_INFO << "SetReportSemaphoreD (data = " << std::hex << data << ")";
                break;
            case Register::SetTesselationProgram:
                PLOG_INFO << "SetTesselationProgram (data = " << std::hex << data << ")";
                break;
            case Register::SetTesselationProgramRegion:
                PLOG_INFO << "SetTesselationProgramRegion (data = " << std::hex << data << ")";
                break;
            case Register::SetFragmentProgram:
                PLOG_INFO << "SetFragmentProgram (data = " << std::hex << data << ")";
                break;
            case Register::SetFragmentProgramRegion:
                PLOG_INFO << "SetFragmentProgramRegion (data = " << std::hex << data << ")";
                break;
            case Register::SetFragmentProgramRegisterCount:
                PLOG_INFO << "SetFragmentProgramRegisterCount (data = " << std::hex << data << ")";
                break;
            case Register::SetFragmentProgramBindGroup:
                PLOG_INFO << "SetFragmentProgramBindGroup (data = " << std::hex << data << ")";
                break;
            case Register::SetConstantBufferSelectorA:
                PLOG_INFO << "SetConstantBufferSelectorA (data = " << std::hex << data << ")";
                break;
            case Register::SetConstantBufferSelectorB:
                PLOG_INFO << "SetConstantBufferSelectorB (data = " << std::hex << data << ")";
                break;
            case Register::SetConstantBufferSelectorC:
                PLOG_INFO << "SetConstantBufferSelectorC (data = " << std::hex << data << ")";
                break;
            case Register::LoadConstantBufferOffset:
                PLOG_INFO << "LoadConstantBufferOffset (data = " << std::hex << data << ")";
                break;
            case Register::LoadConstantBuffer:
                PLOG_INFO << "LoadConstantBuffer (data = " << std::hex << data << ")";
                break;
            case Register::SetColorClamp:
                PLOG_INFO << "SetColorClamp (data = " << std::hex << data << ")";
                break;
            case Register::SetBindlessTexture:
                PLOG_INFO << "SetBindlessTexture (data = " << std::hex << data << ")";
                break;
            default:
                if ((addr >= Register::SetScissorEnable) && (addr < (Register::SetScissorEnable + 4 * NUM_SCISSOR_AREAS))) {
                    switch (addr & 3) {
                        case 0:
                            PLOG_INFO << "SetScissorEnable" << (addr - Register::SetScissorEnable) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetScissorHorizontal" << (addr - Register::SetScissorHorizontal) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetScissorVertical" << (addr - Register::SetScissorVertical) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        default:
                            PLOG_WARNING << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                            break;
                    }
                } else if ((addr >= Register::SetColorTargetA) && (addr < (Register::SetColorTargetA + 16 * NUM_COLOR_TARGETS))) {
                    switch (addr & 15) {
                        case 0:
                            PLOG_INFO << "SetColorTargetA" << (addr - Register::SetColorTargetA) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetColorTargetB" << (addr - Register::SetColorTargetB) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetColorTargetWidth" << (addr - Register::SetColorTargetWidth) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "SetColorTargetHeight" << (addr - Register::SetColorTargetHeight) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 4:
                            PLOG_INFO << "SetColorTargetFormat" << (addr - Register::SetColorTargetFormat) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 5:
                            PLOG_INFO << "SetColorTargetMemory" << (addr - Register::SetColorTargetMemory) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 6:
                            PLOG_INFO << "SetColorTargetThirdDimension" << (addr - Register::SetColorTargetThirdDimension) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 7:
                            PLOG_INFO << "SetColorTargetArrayPitch" << (addr - Register::SetColorTargetArrayPitch) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 8:
                            PLOG_INFO << "SetColorTargetLayer" << (addr - Register::SetColorTargetLayer) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 9:
                            PLOG_INFO << "SetColorTargetMark" << (addr - Register::SetColorTargetMark) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        default:
                            PLOG_WARNING << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                            break;
                    }
                } else if ((addr >= Register::SetViewportScaleX) && (addr < (Register::SetViewportScaleX + 8 * NUM_VIEWPORTS))) {
                    switch (addr & 7) {
                        case 0:
                            PLOG_INFO << "SetViewportScaleX" << (addr - Register::SetViewportScaleX) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetViewportScaleY" << (addr - Register::SetViewportScaleY) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetViewportScaleZ" << (addr - Register::SetViewportScaleZ) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "SetViewportOffsetX" << (addr - Register::SetViewportOffsetX) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 4:
                            PLOG_INFO << "SetViewportOffsetY" << (addr - Register::SetViewportOffsetY) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 5:
                            PLOG_INFO << "SetViewportOffsetZ" << (addr - Register::SetViewportOffsetZ) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 6:
                            PLOG_INFO << "SetViewportCoordinateSwizzle" << (addr - Register::SetViewportCoordinateSwizzle) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 7:
                            PLOG_INFO << "SetViewportIncreaseSnapGridPrecision" << (addr - Register::SetViewportIncreaseSnapGridPrecision) / 8 << " (data = " << std::hex << data << ")";
                            break;
                    }
                } else if ((addr >= Register::SetViewportClipHorizontal) && (addr < (Register::SetViewportClipHorizontal + 4 * NUM_VIEWPORTS))) {
                    switch (addr & 3) {
                        case 0:
                            PLOG_INFO << "SetViewportClipHorizontal" << (addr - Register::SetViewportClipHorizontal) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetViewportClipVertical" << (addr - Register::SetViewportClipVertical) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetViewportClipMinZ" << (addr - Register::SetViewportClipMinZ) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "SetViewportClipMaxZ" << (addr - Register::SetViewportClipMaxZ) / 4 << " (data = " << std::hex << data << ")";
                            break;
                    }
                } else if ((addr >= Register::SetWindowClipHorizonzal) && (addr < (Register::SetWindowClipHorizonzal + 2 * NUM_WINDOWS))) {
                    if ((addr & 1) == 0) {
                        PLOG_INFO << "SetWindowClipHorizonzal" << (addr - Register::SetWindowClipHorizonzal) / 2 << " (data = " << std::hex << data << ")";
                    } else {
                        PLOG_INFO << "SetWindowClipVertical" << (addr - Register::SetWindowClipVertical) / 2 << " (data = " << std::hex << data << ")";
                    }
                } else if ((addr >= Register::SetPolygonStipplePattern) && (addr < (Register::SetPolygonStipplePattern + NUM_POLY_STIPPLE_PATTERNS))) {
                    PLOG_INFO << "SetPolygonStipplePattern" << addr - Register::SetPolygonStipplePattern << " (data = " << std::hex << data << ")";
                } else if ((addr >= Register::SetColorCompression) && (addr < Register::SetCtWrite)) {
                    PLOG_INFO << "SetColorCompression" << addr - Register::SetColorCompression << " (data = " << std::hex << data << ")";
                } else if ((addr >= Register::SetCtWrite) && (addr < (Register::SetCtWrite + NUM_COLOR_TARGETS))) {
                    PLOG_INFO << "SetCtWrite" << addr - Register::SetCtWrite << " (data = " << std::hex << data << ")";
                } else if ((addr >= Register::SetVertexStreamAFormat) && (addr < (Register::SetVertexStreamAFormat + 4 * NUM_VERTEX_STREAMS))) {
                    switch (addr & 3) {
                        case 0:
                            PLOG_INFO << "SetVertexStreamAFormat" << (addr - Register::SetVertexStreamAFormat) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetVertexStreamALocationA" << (addr - Register::SetVertexStreamALocationA) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetVertexStreamALocationB" << (addr - Register::SetVertexStreamALocationB) / 4 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "SetVertexStreamAFrequency" << (addr - Register::SetVertexStreamAFrequency) / 4 << " (data = " << std::hex << data << ")";
                            break;
                    }
                } else if ((addr >= Register::SetVertexStreamLimitAA) && (addr < (Register::SetVertexStreamLimitAA + 2 * NUM_VERTEX_STREAMS))) {
                    switch (addr & 1) {
                        case 0:
                            PLOG_INFO << "SetVertexStreamLimitAA" << (addr - Register::SetVertexStreamLimitAA) / 2 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetVertexStreamLimitAB" << (addr - Register::SetVertexStreamLimitAB) / 2 << " (data = " << std::hex << data << ")";
                            break;
                    }
                } else if ((addr >= Register::SetPipelineShader) && (addr < Register::SetTesselationProgram)) {
                    switch (addr & 15) {
                        case 0:
                            PLOG_INFO << "SetPipelineShader" << (addr - Register::SetPipelineShader) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "SetPipelineProgram" << (addr - Register::SetPipelineProgram) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "SetPipelineReservedA" << (addr - Register::SetPipelineReservedA) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "SetPipelineRegisterCount" << (addr - Register::SetPipelineRegisterCount) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 4:
                            PLOG_INFO << "SetPipelineBinding" << (addr - Register::SetPipelineBinding) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 5:
                            PLOG_INFO << "SetPipelineReservedB" << (addr - Register::SetPipelineReservedB) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 6:
                            PLOG_INFO << "SetPipelineReservedC" << (addr - Register::SetPipelineReservedC) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 7:
                            PLOG_INFO << "SetPipelineReservedD" << (addr - Register::SetPipelineReservedD) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        case 8:
                            PLOG_INFO << "SetPipelineReservedE" << (addr - Register::SetPipelineReservedE) / 16 << " (data = " << std::hex << data << ")";
                            break;
                        default:
                            PLOG_WARNING << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                            break;
                    }
                } else if ((addr >= Register::BindGroupReservedA) && (addr < (Register::BindGroupReservedA + 8 * NUM_BIND_GROUPS))) {
                    switch (addr & 7) {
                        case 0:
                            PLOG_INFO << "BindGroupReservedA" << (addr - Register::BindGroupReservedA) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 1:
                            PLOG_INFO << "BindGroupReservedB" << (addr - Register::BindGroupReservedB) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 2:
                            PLOG_INFO << "BindGroupReservedC" << (addr - Register::BindGroupReservedC) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 3:
                            PLOG_INFO << "BindGroupReservedD" << (addr - Register::BindGroupReservedD) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        case 4:
                            PLOG_INFO << "BindGroupConstantBuffer" << (addr - Register::BindGroupConstantBuffer) / 8 << " (data = " << std::hex << data << ")";
                            break;
                        default:
                            PLOG_WARNING << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                            break;
                    }
                } else if ((addr >= Register::CallMmeMacro) && (addr < (Register::CallMmeMacro + 2 * NUM_MME_REGISTERS))) {
                    if ((addr & 1) == 0) {
                        PLOG_INFO << "CallMmeMacro" << (addr - Register::CallMmeMacro) / 2 << " (data = " << std::hex << data << ")";

                        PLOG_WARNING << "Unimplement MME macro call";
                    } else {
                        PLOG_INFO << "CallMmeData" << (addr - Register::CallMmeData) / 2 << " (data = " << std::hex << data << ")";
                    }
                } else {
                    PLOG_FATAL << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                    exit(0);
                }
        }
    }
}

void writeDMA(u32 addr, u32 data) {
    if (addr > NUM_DMA_REGS) {
        PLOG_FATAL << "Invalid DMA register address " << std::hex << addr;

        exit(0);
    }

    dmaRegs[addr] = data;

    if constexpr (ENABLE_WRITE_LOG) {
        switch (addr) {
            default:
                PLOG_WARNING << "Unrecognized DMA write (register = " << std::hex << addr << ", data = " << data << ")";

                break;
        }
    }
}

}
