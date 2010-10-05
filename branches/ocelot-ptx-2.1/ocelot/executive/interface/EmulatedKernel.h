/*! \file EmulatedKernel.h
	\author Andrew Kerr <arkerr@gatech.edu>
	\date Jan 19, 2009
	\brief implements a kernel emulated on the host CPU
*/

#ifndef EXECUTIVE_EMULATEDKERNEL_H_INCLUDED
#define EXECUTIVE_EMULATEDKERNEL_H_INCLUDED

#include <ocelot/ir/interface/PTXKernel.h>
#include <ocelot/ir/interface/Texture.h>

#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/executive/interface/CTAContext.h>
#include <ocelot/executive/interface/CooperativeThreadArray.h>
#include <hydrazine/interface/Stringable.h>

namespace trace {
	class TraceGenerator;
}

namespace executive {
		
	class EmulatedKernel: public ExecutableKernel {
	public:
		typedef std::deque<ir::PTXInstruction> PTXInstructionVector;
		typedef std::map<int, std::string> ProgramCounterBlockMap;
		typedef std::unordered_map<std::string, int> FunctionNameMap;
		typedef std::unordered_map<int, const EmulatedKernel*> PCToKernelMap;
		typedef CooperativeThreadArray::RegisterFile RegisterFile;

	private:
		static void _computeOffset(const ir::PTXStatement& it, 
			unsigned int& offset, unsigned int& totalOffset);

	public:
		EmulatedKernel(ir::Kernel* kernel, Device* d = 0, 
			bool initialize = true);
		EmulatedKernel(Device *c);
		EmulatedKernel();
		virtual ~EmulatedKernel();
	
		/*!	\brief Determines whether kernel is executable */
		bool executable();
		
		/*!	Launch a kernel on a 2D grid */
		void launchGrid(int width, int height);
	
		/*!	Sets the shape of a kernel */
		void setKernelShape(int x, int y, int z);

		ir::Dim3 getKernelShape() const;
		
		/*! \brief Changes the amount of external shared memory */
		void setExternSharedMemorySize(unsigned int bytes);
		
		/*!	Sets device used to execute the kernel */
		void setWorkerThreads(unsigned int limit);

		/*! \brief Indicate that the kernels parameters have been updated */
		void updateArgumentMemory();
		
		/*! \brief Indicate that other memory has been updated */
		void updateMemory();

		/*! \brief Get a vector of all textures references by the kernel */
		TextureVector textureReferences() const;

	public:
		/*!	adds a trace generator to the EmulatedKernel */
		void addTraceGenerator(trace::TraceGenerator *generator);
		
		/*!	removes a trace generator from an EmulatedKernel */
		void removeTraceGenerator(trace::TraceGenerator *generator);

		/*! \brief Initialize the kernel */
		void initialize();

		/*!	Maps identifiers to global memory allocations. */
		void initializeGlobalMemory();
		
		/*! Lazily sets the target of a call instruction to the entry point
			of the specified function.  This function will be inserted into
			the instruction sequence if it does not already exist */
		void lazyLink(int callPC, const std::string& functionName);

		/*! Finds the kernel beginning at the specified PC */
		const EmulatedKernel* getKernel(int PC) const;
		
		/*! If the kernel is executing, jump to the specified PC */
		void jumpToPC(int PC);

		/* Get a snapshot of the current register file */
		RegisterFile getCurrentRegisterFile() const;

		/* Get a pointer to the base of the current shared memory block */
		const char* getSharedMemory() const;

	protected:
		/*! Cleans up the EmulatedKernel instance*/
		void freeAll();

		/*!	On construction, allocates registers by computing live ranges */
		void registerAllocation();

		/*!	Produces a packed vector of instructions, updates each operand, 
			and changes labels to indices.
		*/
		void constructInstructionSequence();

		/*!	After emitting the instruction sequence, visit each memory move 
			operation and replace references to parameters with offsets into 
			parameter memory.
		*/
		void updateParamReferences();

		/*!	Allocate parameter memory*/	
		void initializeParameterMemory();

		/*!	Allocates arrays in shared memory and maps identifiers to 
			allocations. */
		void initializeSharedMemory();

		/*!	Allocates arrays in local memory and maps identifiers to 
			allocations. */
		void initializeLocalMemory();

		/*!	Maps identifiers to const memory allocations. */
		void initializeConstMemory();

		/*!	Maps identifiers to global shared memory allocations. */
		void initializeGlobalSharedMemory();
		
		/*! Determines stack memory size and maps identifiers to allocations */
		void initializeStackMemory();
		
		/*!	Scans the kernel and builds the set of textures using references 
				in tex instructions */
		void initializeTextureMemory();

		/*! Sets the target of call instructions to invalid pcs so that they
			can be lazily compiled and allocated */
		void invalidateCallTargets();

	public:
		/*! A map of register name to register number */
		ir::PTXKernel::RegisterMap registerMap;

		/*!	Pointer to block of memory used to store parameter data */
		char* ParameterMemory;

		/*!	Pointer to byte-addressable const memory */
		char* ConstMemory;

		/*!	Packed and allocated vector of instructions */
		PTXInstructionVector instructions;

		/*! Maps program counters of header instructions to basic block label */
		ProgramCounterBlockMap branchTargetsToBlock;
		
		/*! maps the program counter of the terminating instructions to owning basic block */
		ProgramCounterBlockMap basicBlockMap;
		
		/*! maps a PC to the basic block it starts */
		ProgramCounterBlockMap basicBlockPC;

		/*!	Packed vector of mapped textures */
		TextureVector textures;

	private:
		/*! Maps program counter to the kernel that begins there */
		PCToKernelMap kernelEntryPoints;

		/*! A map of function names to the PC of their entry point */
		FunctionNameMap functionEntryPoints;

		/*! A handle to the current CTA, or 0 if none is executing */
		executive::CooperativeThreadArray* CTA;

	public:
		/*! \brief Check to see if a memory access is valid */
		bool checkMemoryAccess(const void* base, size_t size) const;
	
	public:
		/*! Copies data from global objects into const and global memory */
		void updateGlobals();

	public:
		/*!	Print out every instruction	*/
		std::string toString() const;
		
		/*! \brief Get the file name that the kernel resides in */
		std::string fileName() const;
		
		/*! \brief Get the nearest location to an instruction at a given PC */
		std::string location(unsigned int PC) const;
		
		/*!	\brief gets the basic block label owning the instruction 
			specified by the PC */
		std::string getInstructionBlock(int PC) const;
	};

}

#endif
