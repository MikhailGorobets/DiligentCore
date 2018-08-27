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
/// Defines Diligent::IBuffer interface and related data structures

#include "DeviceObject.h"

namespace Diligent
{

// {EC47EAD3-A2C4-44F2-81C5-5248D14F10E4}
static constexpr INTERFACE_ID IID_Buffer =
{ 0xec47ead3, 0xa2c4, 0x44f2, { 0x81, 0xc5, 0x52, 0x48, 0xd1, 0x4f, 0x10, 0xe4 } };

/// Describes the buffer access mode.

/// This enumeration is used by BufferDesc structure.
enum BUFFER_MODE : Uint8
{
    /// Undefined mode.
    BUFFER_MODE_UNDEFINED = 0,

    /// Formated buffer. Access to the buffer will use format conversion operations.
    /// In this mode, ElementByteStride member of BufferDesc defines the buffer element size.
    /// Buffer views can use different formats, but the format size must match ElementByteStride.
    BUFFER_MODE_FORMATTED,
        
    /// Structured buffer.
    /// In this mode, ElementByteStride member of BufferDesc defines the structure stride.
    BUFFER_MODE_STRUCTURED,

    /// Raw buffer.
    /// In this mode, the buffer is accessed as raw bytes. Formatted views of a raw
    /// buffer can also be created similar to formatted buffer. If formatted views
    /// are to be created, the ElementByteStride member of BufferDesc must specify the 
    /// size of the format.
    BUFFER_MODE_RAW,

    /// Helper value storing the total number of modes in the enumeration.
    BUFFER_MODE_NUM_MODES
};

/// Buffer description
struct BufferDesc : DeviceObjectAttribs
{
    /// Size of the buffer, in bytes. For a uniform buffer, this must be multiple of 16.
    Uint32 uiSizeInBytes = 0;

    /// Buffer bind flags, see Diligent::BIND_FLAGS for details

    /// The following bind flags are allowed:
    /// Diligent::BIND_VERTEX_BUFFER, Diligent::BIND_INDEX_BUFFER, Diligent::BIND_UNIFORM_BUFFER,
    /// Diligent::BIND_SHADER_RESOURCE, Diligent::BIND_STREAM_OUTPUT, Diligent::BIND_UNORDERED_ACCESS,
    /// Diligent::BIND_INDIRECT_DRAW_ARGS
    Uint32 BindFlags = 0;

    /// Buffer usage, see Diligent::USAGE for details
    USAGE Usage = USAGE_DEFAULT;

    /// CPU access flags or 0 if no CPU access is allowed, 
    /// see Diligent::CPU_ACCESS_FLAG for details.
    Uint8 CPUAccessFlags = 0;
    
    /// Buffer mode, see Diligent::BUFFER_MODE
    BUFFER_MODE Mode = BUFFER_MODE_UNDEFINED;

    /// Buffer element stride, in bytes.

    /// For a structured buffer (BufferDesc::Mode equals Diligent::BUFFER_MODE_STRUCTURED) this member 
    /// defines the size of each buffer element. For a formatted buffer
    /// (BufferDesc::Mode equals Diligent::BUFFER_MODE_FORMATTED) and optionally for a raw buffer 
    /// (Diligent::BUFFER_MODE_RAW), this member defines the size of the format that will be used for views 
    /// created for this buffer.
    Uint32 ElementByteStride = 0;

    /// Tests if two structures are equivalent

    /// \param [in] RHS - reference to the structure to perform comparison with
    /// \return 
    /// - True if all members of the two structures except for the Name are equal.
    /// - False otherwise.
    /// The operator ignores DeviceObjectAttribs::Name field as it does not affect 
    /// the buffer description.
    bool operator == (const BufferDesc& RHS)const
    {
        return uiSizeInBytes     == RHS.uiSizeInBytes  && 
               BindFlags         == RHS.BindFlags      &&
               Usage             == RHS.Usage          &&
               CPUAccessFlags    == RHS.CPUAccessFlags &&
               Mode              == RHS.Mode           &&
               ElementByteStride == RHS.ElementByteStride;
    }
};

/// Describes the buffer initial data
struct BufferData
{
    /// Pointer to the data
    const void* pData;

    /// Data size, in bytes
    Uint32 DataSize;

    /// Initializes the structure members with default values

    /// Default values:
    /// Member              | Default value
    /// --------------------|--------------
    /// pData               | nullptr
    /// DataSize            | 0
    BufferData() : 
        pData(nullptr),
        DataSize(0)
    {}
};

/// Buffer interface

/// Defines the methods to manipulate a buffer object
class IBuffer : public IDeviceObject
{
public:
    /// Queries the specific interface, see IObject::QueryInterface() for details
    virtual void QueryInterface( const Diligent::INTERFACE_ID &IID, IObject **ppInterface ) = 0;

    /// Returns the buffer description used to create the object
    virtual const BufferDesc& GetDesc()const = 0;
    
    /// Updates the data in the buffer

    /// \param [in] pContext - Pointer to the device context interface to be used to perform the operation.
    /// \param [in] Offset - Offset in bytes from the beginning of the buffer to the update region.
    /// \param [in] Size - Size in bytes of the data region to update.
    /// \param [in] pData - Pointer to the data to store in the buffer.
    virtual void UpdateData( class IDeviceContext *pContext, Uint32 Offset, Uint32 Size, const PVoid pData) = 0;

    /// Copies the data from other buffer

    /// \param [in] pContext - Pointer to the device context interface to be used to perform the operation.
    /// \param [in] pSrcBuffer - Source buffer to copy data from.
    /// \param [in] SrcOffset - Offset in bytes from the beginning of the source buffer to the beginning of data to copy.
    /// \param [in] DstOffset - Offset in bytes from the beginning of the destination buffer to the beginning 
    ///                         of the destination region.
    /// \param [in] Size    - Size in bytes of data to copy.
    virtual void CopyData( IDeviceContext *pContext, IBuffer *pSrcBuffer, Uint32 SrcOffset, Uint32 DstOffset, Uint32 Size ) = 0;
    
    /// Maps the buffer

    /// \param [in] pContext - Pointer to the device context interface to be used to perform the operation.
    /// \param [in] MapType - Type of the map operation. See Diligent::MAP_TYPE.
    /// \param [in] MapFlags - Special map flags. See Diligent::MAP_FLAGS.
    /// \param [out] pMappedData - Reference to the void pointer to store the address of the mapped region.
    virtual void Map( IDeviceContext *pContext, MAP_TYPE MapType, Uint32 MapFlags, PVoid &pMappedData ) = 0;

    /// Unmaps the previously mapped buffer
    /// \param [in] pContext - Pointer to the device context interface to be used to perform the operation.
    /// \param [in] MapType - Type of the map operation. This parameter must match the type that was 
    ///                       provided to the Map() method. 
    /// \param [in] MapFlags - Map flags. This parameter must match the flags that were provided to 
    ///                        the Map() method. 
    virtual void Unmap( IDeviceContext *pContext, MAP_TYPE MapType, Uint32 MapFlags ) = 0;

    /// Creates a new buffer view

    /// \param [in] ViewDesc - View description. See Diligent::BufferViewDesc for details.
    /// \param [out] ppView - Address of the memory location where the pointer to the view interface will be written to.
    /// 
    /// \remarks To create a view addressing the entire buffer, set only BufferViewDesc::ViewType member
    ///          of the ViewDesc structure and leave all other members in their default values.\n
    ///          Buffer view will contain strong reference to the buffer, so the buffer will not be destroyed
    ///          until all views are released.\n
    ///          The function calls AddRef() for the created interface, so it must be released by
    ///          a call to Release() when it is no longer needed.
    virtual void CreateView( const struct BufferViewDesc &ViewDesc, class IBufferView **ppView ) = 0;

    /// Returns the pointer to the default view.
    
    /// \param [in] ViewType - Type of the requested view. See Diligent::BUFFER_VIEW_TYPE.
    /// \return Pointer to the interface
    ///
    /// \remarks Default views are only created for structured and raw buffers. As for formatted buffers
    ///          the view format is unknown at buffer initialization time, no default views are created.
    ///
    /// \note The function does not increase the reference counter for the returned interface, so
    ///       Release() must *NOT* be called.
    virtual IBufferView* GetDefaultView( BUFFER_VIEW_TYPE ViewType ) = 0;

    /// Returns native buffer handle specific to the underlying graphics API

    /// \return pointer to ID3D11Resource interface, for D3D11 implementation\n
    ///         pointer to ID3D12Resource interface, for D3D12 implementation\n
    ///         GL buffer handle, for GL implementation
    virtual void* GetNativeHandle() = 0;
};

}
