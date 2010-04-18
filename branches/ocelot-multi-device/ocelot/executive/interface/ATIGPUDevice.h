/*! \file ATIGPUDevice.h
 *  \author Rodrigo Dominguez <rdomingu@ece.neu.edu>
 *  \date April 7, 2010
 *  \brief The header file for the ATI GPU Device class.
 */

#ifndef ATIGPUDEVICE_H_INCLUDED
#define ATIGPUDEVICE_H_INCLUDED

// Ocelot includes
#include <ocelot/executive/interface/Device.h>
#include <ocelot/cal/interface/CalDriver.h>

namespace executive
{
	/*! \brief ATI GPU Device */
	class ATIGPUDevice : public Device 
	{
		public:
			/*! \brief ATI memory allocation */
			class MemoryAllocation : public Device::MemoryAllocation
			{
				public:
					/*! \brief Constructor */
					MemoryAllocation(CALdevice device, CALcontext context, 
							CALmodule module, unsigned int uav, size_t size);
					/*! \brief Destructor */
					~MemoryAllocation();

					/*! \brief Get the flags if this is a host pointer */
					unsigned int flags() const;
					/*! \brief Get a host pointer if for a host allocation */
					void *mappedPointer() const;
					/*! \brief Get a device pointer to the base */
					void *pointer() const;
					/*! \brief The size of the allocation */
					size_t size() const;
					/*! \brief Copy from an external host pointer */
					void copy(size_t offset, const void *host, size_t size );
					/*! \brief Copy to an external host pointer */
					void copy(void *host, size_t offset, size_t size) const;
					/*! \brief Memset the allocation */
					void memset(size_t offset, int value, size_t size);
					/*! \brief Copy to another allocation */
					void copy(Device::MemoryAllocation *allocation, 
						size_t toOffset, size_t fromOffset, size_t size) const;

				private:
					/*! \brief CAL resource */
					CALresource _resource;
					/*! \brief CAL Context */
					CALcontext _context;
					/*! \brief CAL memory handle */
					CALmem _mem;
					/*! \brief CAL module name */
					CALname _name;
					/*! \brief Size of the allocation (in byte) */
					size_t _size;
			};

			/*! \brief Allocate a new device for each CAL capable GPU */
			static DeviceVector createDevices(unsigned int flags);
		
			/*! \brief Constructor */
			ATIGPUDevice();
			/*! \brief Destructor */
			~ATIGPUDevice();

			Device::MemoryAllocation *getMemoryAllocation(const void *address, 
				bool hostAllocation) const;
			/*! \brief Get the address of a global by stream */
			Device::MemoryAllocation *getGlobalAllocation(
				const std::string& module, const std::string& name);
			/*! \brief Allocate some new dynamic memory on this device */
			Device::MemoryAllocation *allocate(size_t size);
			/*! \brief Make this a host memory allocation */
			Device::MemoryAllocation *allocateHost(size_t size, 
				unsigned int flags);
			/*! \brief Free an existing non-global allocation */
			void free(void *pointer);
			/*! \brief Get nearby allocations to a pointer */
			Device::MemoryAllocationVector getNearbyAllocations(
				void *pointer) const;
		
			/*! \brief Registers an opengl buffer with a resource */
			void* glRegisterBuffer(unsigned int buffer, 
				unsigned int flags);
			/*! \brief Registers an opengl image with a resource */
			void* glRegisterImage(unsigned int image, 
				unsigned int target, unsigned int flags);
			/*! \brief Unregister a resource */
			void unRegisterGraphicsResource(void* resource);
			/*! \brief Map a graphics resource for use with this device */
			void mapGraphicsResource(void* resource, int count, 
				unsigned int stream);
			/*! \brief Get a pointer to a mapped resource along with its size */
			void* getPointerToMappedGraphicsResource(size_t& size, 
				void* resource) const;
			/*! \brief Change the flags of a mapped resource */
			void setGraphicsResourceFlags(void* resource, 
				unsigned int flags);
			/*! \brief Unmap a mapped resource */
			void unmapGraphicsResource(void* resource);

			/*! \brief Load a module, must have a unique name */
			void load(const ir::Module *irModule);
			/*! \brief Unload a module by name */
			void unload(const std::string& name);

			/*! \brief Create a new event */
			unsigned int createEvent(int flags);
			/*! \brief Destroy an existing event */
			void destroyEvent(unsigned int event);
			/*! \brief Query to see if an event has been recorded (yes/no) */
			bool queryEvent(unsigned int event) const;
			/*! \brief Record something happening on an event */
			void recordEvent(unsigned int event, unsigned int stream);
			/*! \brief Synchronize on an event */
			void synchronizeEvent(unsigned int event);
			/*! \brief Get the elapsed time in ms between two recorded events */
			float getEventTime(unsigned int start, unsigned int end) const;
		
			/*! \brief Create a new stream */
			unsigned int createStream();
			/*! \brief Destroy an existing stream */
			void destroyStream(unsigned int stream);
			/*! \brief Query the status of an existing stream (ready/not) */
			bool queryStream(unsigned int stream) const;
			/*! \brief Synchronize a particular stream */
			void synchronizeStream(unsigned int stream);
			/*! \brief Sets the current stream */
			void setStream(unsigned int stream);
			
			/*! \brief Select this device as the current device.
			 *  Only one device is allowed to be selected at any time. */
			void select();
			/*! \brief Is this device selected? */
			bool selected() const;
			/*! \brief Deselect this device */
			void unselect();
			
			/*! \brief Binds a texture to a memory allocation at a pointer */
			void bindTexture(void* pointer, const std::string& moduleName,
				const std::string& textureName, 
				const textureReference& ref, const cudaChannelFormatDesc& desc, 
				const ir::Dim3& size);
			/*! \brief unbinds anything bound to a particular texture */
			void unbindTexture(const std::string& moduleName, 
				const std::string& textureName);
			/*! \brief Get's a reference to an internal texture */
			void* getTextureReference(const std::string& moduleName, 
				const std::string& textureName);
			
			/*! \brief helper function for launching a kernel
			 *  \param module module name
			 *  \param kernel kernel name
			 *  \param grid grid dimensions
			 *  \param block block dimensions
			 *  \param sharedMemory shared memory size
			 *  \param parameterBlock array of bytes for parameter memory
			 *  \param parameterBlockSize number of bytes in parameter memory
			 *  \param traceGenerators vector of trace generators to add and 
			 	remove from kernel
			 *  \param stream The stream to launch the kernel in
			 */
			void launch(const std::string& module, 
					const std::string& kernel, const ir::Dim3& grid, 
					const ir::Dim3& block, size_t sharedMemory, 
					const void *parameterBlock, size_t parameterBlockSize, 
					const trace::TraceGeneratorVector& 
					traceGenerators = trace::TraceGeneratorVector());
			/*! \brief Get the function attributes of a specific kernel */
			cudaFuncAttributes getAttributes(const std::string& module, 
				const std::string& kernel);
			/*! \brief Get the last error from this device */
			unsigned int getLastError() const;
			/*! \brief Wait until all asynchronous operations have completed */
			void synchronize();

			/*! \brief Limit the worker threads used by this device */
			void limitWorkerThreads(unsigned int threads);

		private:
			/*! \brief A map of memory allocations */
			typedef std::map<void*, MemoryAllocation*> AllocationMap;

			/*! \brief A map of memory allocations in device space */
			AllocationMap _allocations;

			/*! \brief CAL Device */
			CALdevice _device;
			/*! \brief CAL Device Info */
			CALdeviceinfo _info;
			/*! \brief CAL Context. Multiple contexts per device is not supported yet */
			CALcontext _context;
			/*! \brief CAL Object */
			CALobject _object;
			/*! \brief CAL Image */
			CALimage _image;
			/*! \brief CAL Module */
			CALmodule _module;

			/*! \brief Has this device been selected? */
			bool _selected;
			
			/*! \brief Returns a pointer to an instance to the CalDriver singleton */
			static cal::CalDriver *CalDriver();
	};
}

#endif