/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// Declaration of Diligent::RootSignature class
#include "ShaderD3DBase.h"
#include "ShaderResourceLayoutD3D12.h"

namespace Diligent
{

SHADER_TYPE ShaderTypeFromShaderVisibility(D3D12_SHADER_VISIBILITY ShaderVisibility);
D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
D3D12_DESCRIPTOR_HEAP_TYPE dbgHeapTypeFromRangeType(D3D12_DESCRIPTOR_RANGE_TYPE RangeType);

class RootParameter
{
public:

	RootParameter(D3D12_ROOT_PARAMETER_TYPE ParameterType, Uint32 RootIndex, UINT Register, UINT RegisterSpace, D3D12_SHADER_VISIBILITY Visibility, SHADER_VARIABLE_TYPE VarType)  : 
        m_RootIndex(RootIndex),
        m_ShaderVarType(VarType)
	{
        VERIFY(ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV, "Unexpected parameter type - verify argument list");
		m_RootParam.ParameterType = ParameterType;
		m_RootParam.ShaderVisibility = Visibility;
		m_RootParam.Descriptor.ShaderRegister = Register;
		m_RootParam.Descriptor.RegisterSpace = RegisterSpace;
	}

	RootParameter( D3D12_ROOT_PARAMETER_TYPE ParameterType, Uint32 RootIndex, UINT Register, UINT RegisterSpace, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility, SHADER_VARIABLE_TYPE VarType) : 
        m_RootIndex(RootIndex),
        m_ShaderVarType(VarType)
	{
        VERIFY(ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, "Unexpected parameter type - verify argument list");
		m_RootParam.ParameterType = ParameterType;
		m_RootParam.ShaderVisibility = Visibility;
		m_RootParam.Constants.Num32BitValues = NumDwords;
		m_RootParam.Constants.ShaderRegister = Register;
		m_RootParam.Constants.RegisterSpace = RegisterSpace;
	}

	RootParameter( D3D12_ROOT_PARAMETER_TYPE ParameterType, Uint32 RootIndex, UINT NumRanges, D3D12_DESCRIPTOR_RANGE *pRanges, D3D12_SHADER_VISIBILITY Visibility, SHADER_VARIABLE_TYPE VarType) : 
        m_RootIndex(RootIndex),
        m_ShaderVarType(VarType)
	{
        VERIFY(ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Unexpected parameter type - verify argument list");
        VERIFY_EXPR(pRanges != nullptr);
		m_RootParam.ParameterType = ParameterType;
		m_RootParam.ShaderVisibility = Visibility;
		m_RootParam.DescriptorTable.NumDescriptorRanges = NumRanges;
		m_RootParam.DescriptorTable.pDescriptorRanges = pRanges;
#ifdef _DEBUG
        for(Uint32 r=0; r < NumRanges; ++r)
            pRanges[r].RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1);
#endif
	}

    RootParameter(const RootParameter &RP):
        m_RootParam( RP.m_RootParam),
        m_DescriptorTableSize(RP.m_DescriptorTableSize),
        m_ShaderVarType(RP.m_ShaderVarType),
        m_RootIndex(RP.m_RootIndex)
    {
        VERIFY(m_RootParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Use another constructor to copy descriptor table");
    }

    RootParameter(const RootParameter &RP, UINT NumRanges, D3D12_DESCRIPTOR_RANGE *pRanges):
        m_RootParam( RP.m_RootParam),
        m_DescriptorTableSize(RP.m_DescriptorTableSize),
        m_ShaderVarType(RP.m_ShaderVarType),
        m_RootIndex(RP.m_RootIndex)
    {
        VERIFY(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Root parameter is expected to be a descriptor table");
        VERIFY(NumRanges >= m_RootParam.DescriptorTable.NumDescriptorRanges, "New table must be larger than source one");
        auto &DstTbl = m_RootParam.DescriptorTable;
        DstTbl.NumDescriptorRanges = NumRanges;
        DstTbl.pDescriptorRanges = pRanges;
        const auto &SrcTbl = RP.m_RootParam.DescriptorTable;
        memcpy(pRanges, SrcTbl.pDescriptorRanges, SrcTbl.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE));
#ifdef _DEBUG
        {
            Uint32 dbgTableSize = 0;
            for (Uint32 r = 0; r < SrcTbl.NumDescriptorRanges; ++r)
            {
                const auto &Range = SrcTbl.pDescriptorRanges[r];
                dbgTableSize = std::max(dbgTableSize, Range.OffsetInDescriptorsFromTableStart + Range.NumDescriptors);
            }
            VERIFY(dbgTableSize == m_DescriptorTableSize, "Incorrect descriptor table size");
            
            for (Uint32 r = SrcTbl.NumDescriptorRanges; r < DstTbl.NumDescriptorRanges; ++r)
                pRanges[r].RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1);
        }
#endif
    }

    RootParameter& operator = (const RootParameter &RP) = delete;
    RootParameter& operator = (RootParameter &&RP) = delete;

    void SetDescriptorRange( UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space = 0, UINT OffsetFromTableStart =  D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
	{
        VERIFY(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Incorrect parameter table: descriptor table is expected");
        auto &Tbl = m_RootParam.DescriptorTable;
        VERIFY(RangeIndex < Tbl.NumDescriptorRanges, "Invalid descriptor range index");
		D3D12_DESCRIPTOR_RANGE &range = const_cast<D3D12_DESCRIPTOR_RANGE &>(Tbl.pDescriptorRanges[RangeIndex]);
        VERIFY(range.RangeType == static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1), "Descriptor range has already been initialized. m_DescriptorTableSize may be updated incorrectly");
		range.RangeType = Type;
		range.NumDescriptors = Count;
		range.BaseShaderRegister = Register;
		range.RegisterSpace = Space;
		range.OffsetInDescriptorsFromTableStart = OffsetFromTableStart;
        m_DescriptorTableSize = std::max(m_DescriptorTableSize, OffsetFromTableStart + Count);
	}

    SHADER_VARIABLE_TYPE GetShaderVariableType()const{ return m_ShaderVarType; }
    Uint32 GetDescriptorTableSize()const
    {
        VERIFY(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Incorrect parameter table: descriptor table is expected");
        return m_DescriptorTableSize;
    }
    D3D12_SHADER_VISIBILITY GetShaderVisibility()const{return m_RootParam.ShaderVisibility;}
    D3D12_ROOT_PARAMETER_TYPE GetParameterType()const{return m_RootParam.ParameterType;}

    Uint32 GetRootIndex()const{return m_RootIndex;}

	operator const D3D12_ROOT_PARAMETER&()const{return m_RootParam;}

private:

    SHADER_VARIABLE_TYPE m_ShaderVarType = static_cast<SHADER_VARIABLE_TYPE>(-1);
    D3D12_ROOT_PARAMETER m_RootParam = {};
    Uint32 m_DescriptorTableSize = 0;
    Uint32 m_RootIndex = static_cast<Uint32>(-1);
};



/// Implementation of the Diligent::RootSignature class
class RootSignature
{
public:
    RootSignature();

    void AllocateStaticSamplers(IShader* const *ppShaders, Uint32 NumShaders);

    void Finalize(ID3D12Device *pd3d12Device);

    ID3D12RootSignature* GetD3D12RootSignature()const{return m_pd3d12RootSignature;}
    void InitResourceCache(class RenderDeviceD3D12Impl *pDeviceD3D12Impl, class ShaderResourceCacheD3D12& ResourceCache, IMemoryAllocator &CacheMemAllocator)const;
    
    void InitStaticSampler(SHADER_TYPE ShaderType, const String &TextureName, const D3DShaderResourceAttribs &ShaderResAttribs);

    void AllocateResourceSlot(SHADER_TYPE ShaderType, const D3DShaderResourceAttribs &ShaderResAttribs, D3D12_DESCRIPTOR_RANGE_TYPE RangeType, Uint32 &RootIndex, Uint32 &OffsetFromTableStart);

    // This method should be thread-safe as it does not modify any object state
    void (RootSignature::*CommitDescriptorHandles)(class RenderDeviceD3D12Impl *pRenderDeviceD3D12, 
                                                   ShaderResourceCacheD3D12& ResourceCache, 
                                                   class CommandContext &Ctx, 
                                                   bool IsCompute)const = nullptr;

    void (RootSignature::*TransitionAndCommitDescriptorHandles)(class RenderDeviceD3D12Impl *pRenderDeviceD3D12, 
                                                                ShaderResourceCacheD3D12& ResourceCache, 
                                                                class CommandContext &Ctx, 
                                                                bool IsCompute)const = nullptr;

    void TransitionResources(ShaderResourceCacheD3D12& ResourceCache, 
                             class CommandContext &Ctx)const;

    void CommitRootViews(ShaderResourceCacheD3D12& ResourceCache, 
                         class CommandContext &Ctx, 
                         bool IsCompute,
                         Uint32 ContextId)const;

    Uint32 GetTotalSrvCbvUavSlots(SHADER_VARIABLE_TYPE VarType)const
    {
        VERIFY_EXPR(VarType >= 0 && VarType < SHADER_VARIABLE_TYPE_NUM_TYPES);
        return m_TotalSrvCbvUavSlots[VarType];
    }
    Uint32 GetTotalSamplerSlots(SHADER_VARIABLE_TYPE VarType)const
    {
        VERIFY_EXPR(VarType >= 0 && VarType < SHADER_VARIABLE_TYPE_NUM_TYPES);
        return m_TotalSamplerSlots[VarType];
    }

private:
#ifdef _DEBUG
    void dbgVerifyRootParameters()const;
#endif

    Uint32 m_TotalSrvCbvUavSlots[SHADER_VARIABLE_TYPE_NUM_TYPES];
    Uint32 m_TotalSamplerSlots[SHADER_VARIABLE_TYPE_NUM_TYPES];
    
    CComPtr<ID3D12RootSignature> m_pd3d12RootSignature;

    class RootParamsManager
    {
    public:
        RootParamsManager(IMemoryAllocator &MemAllocator);

        RootParamsManager(const RootParamsManager&) = delete;
        RootParamsManager& operator= (const RootParamsManager&) = delete;
        RootParamsManager(RootParamsManager&&) = delete;
        RootParamsManager& operator= (RootParamsManager&&) = delete;
        
        Uint32 GetNumRootTables()const{return m_NumRootTables;}
        Uint32 GetNumRootViews()const{return m_NumRootViews;}
        
        const RootParameter& GetRootTable(Uint32 TableInd)const
        {
            VERIFY_EXPR(TableInd < m_NumRootTables);
            return m_pRootTables[TableInd];
        }

        RootParameter& GetRootTable(Uint32 TableInd)
        {
            VERIFY_EXPR(TableInd < m_NumRootTables);
            return m_pRootTables[TableInd];
        }

        const RootParameter& GetRootView(Uint32 ViewInd)const
        {
            VERIFY_EXPR(ViewInd < m_NumRootViews);
            return m_pRootViews[ViewInd];
        }

        RootParameter& GetRootView(Uint32 ViewInd)
        {
            VERIFY_EXPR(ViewInd < m_NumRootViews);
            return m_pRootViews[ViewInd];
        }

        void AddRootView(D3D12_ROOT_PARAMETER_TYPE ParameterType, Uint32 RootIndex, UINT Register, D3D12_SHADER_VISIBILITY Visibility, SHADER_VARIABLE_TYPE VarType);
        void AddRootTable(Uint32 RootIndex, D3D12_SHADER_VISIBILITY Visibility, SHADER_VARIABLE_TYPE VarType, Uint32 NumRangesInNewTable = 1);
        void AddDescriptorRanges(Uint32 RootTableInd, Uint32 NumExtraRanges = 1);

        template<class TOperation>
        void ProcessRootTables(TOperation)const;

    private:
        size_t GetRequiredMemorySize(Uint32 NumExtraRootTables, Uint32 NumExtraRootViews, Uint32 NumExtraDescriptorRanges)const;
        D3D12_DESCRIPTOR_RANGE* Extend(Uint32 NumExtraRootTables, Uint32 NumExtraRootViews, Uint32 NumExtraDescriptorRanges, Uint32 RootTableToAddRanges = static_cast<Uint32>(-1));

        IMemoryAllocator &m_MemAllocator;
        std::unique_ptr<void, STDDeleter<void, IMemoryAllocator>> m_pMemory;
        Uint32 m_NumRootTables = 0;
        Uint32 m_NumRootViews = 0;
        Uint32 m_TotalDescriptorRanges = 0;
        RootParameter *m_pRootTables = nullptr;
        RootParameter *m_pRootViews = nullptr;
    };

    static constexpr Uint8 InvalidRootTableIndex = static_cast<Uint8>(-1);
    
    // The array below contains array index of a CBV/SRV/UAV root table 
    // in m_RootParams (NOT the Root Index!), for every variable type 
    // (static, mutable, dynamic) and every shader type,
    // or -1, if the table is not yet assigned to the combination
    Uint8 m_SrvCbvUavRootTablesMap[SHADER_VARIABLE_TYPE_NUM_TYPES * 6];
    // This array contains the same data for Sampler root table
    Uint8 m_SamplerRootTablesMap[SHADER_VARIABLE_TYPE_NUM_TYPES * 6];

    RootParamsManager m_RootParams;
    
    struct StaticSamplerAttribs
    {
        StaticSamplerDesc SamplerDesc;
        UINT ShaderRegister = static_cast<UINT>(-1);
        UINT ArraySize = 0;
        UINT RegisterSpace = 0;
        D3D12_SHADER_VISIBILITY ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(-1);
        
        StaticSamplerAttribs(){}
        StaticSamplerAttribs(const StaticSamplerDesc& SamDesc, D3D12_SHADER_VISIBILITY Visibility) : 
            SamplerDesc(SamDesc),
            ShaderVisibility(Visibility)
        {}
    };
    // Note: sizeof(m_StaticSamplers) == 56 (MS compiler, release x64)
    std::vector<StaticSamplerAttribs, STDAllocatorRawMem<StaticSamplerAttribs> > m_StaticSamplers;

    IMemoryAllocator &m_MemAllocator;

    // Commits descriptor handles for static and mutable variables
    template<bool PerformResourceTransitions>
    void CommitDescriptorHandlesInternal_SM(class RenderDeviceD3D12Impl *pRenderDeviceD3D12, 
                                            ShaderResourceCacheD3D12& ResourceCache, 
                                            class CommandContext &Ctx, 
                                            bool IsCompute)const;
    template<bool PerformResourceTransitions>
    // Commits descriptor handles for static, mutable, and dynamic variables
    void CommitDescriptorHandlesInternal_SMD(class RenderDeviceD3D12Impl *pRenderDeviceD3D12, 
                                            ShaderResourceCacheD3D12& ResourceCache, 
                                            class CommandContext &Ctx, 
                                            bool IsCompute)const;
};

}
