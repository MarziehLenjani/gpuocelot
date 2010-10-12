/*! \file ExecuteInstrumentedKernel.cpp
	\date Monday October 4, 2010
	\author Naila Farooqui <naila@cc.gatech.edu>
	\brief The source file for the ExecuteInstrumentedKernel class.
*/

#ifndef EXECUTE_INSTRUMENTED_KERNEL_CPP_INCLUDED
#define EXECUTE_INSTRUMENTED_KERNEL_CPP_INCLUDED

#include <ocelot/analysis/test/ExecuteInstrumentedKernel.h>
#include <ocelot/api/interface/ocelot.h>

#include <ocelot/cuda/interface/cuda_runtime.h>

#include <hydrazine/interface/Thread.h>
#include <hydrazine/implementation/ArgumentParser.h>
#include <hydrazine/implementation/debug.h>

#include <fstream>

namespace test
{

    /* NOTE: This class right now is really a utility rather than
        an actual test file. Basically, I read in the instrumented PTX
        file and output the results just to see what the basic block 
        execution count looks like. I need to write proper tests for
        checking that the basic block count is correct. Working on 
        that. */
   
    bool ExecuteInstrumentedKernel::execute()
	{ 
		ocelot::reset();

        std::ifstream file(input);
        if(!file.is_open()) {
            std::cout << "Could not open input file.\n";
            return false;
        }
        
        ocelot::registerPTXModule(file, moduleName);        		
        
    	cudaConfigureCall( dim3( 1, 1, 1 ), dim3( 1, 1, 1 ), 0, 0 );
        
		ocelot::launch( moduleName, kernelName );
		
        size_t *counter;
        cudaGetSymbolAddress((void **) &counter, "__ocelot_basic_block_counter");
        cudaMemcpyToSymbol("counter", &counter, sizeof( size_t ), 0, cudaMemcpyDeviceToHost);
		
        std::cout << "\n--------------- Basic Block Execution Count ---------------\n\n";

        for(int i = 0; counter[i] != 0; i++) {
            std::cout << "basic block " << (i + 1) << ": " << counter[i] << std::endl;
        }

		bool pass = true;
		status << "Test Passed\n";		

		return pass;
	}

	bool ExecuteInstrumentedKernel::doTest()
	{
		return execute();
	}

	ExecuteInstrumentedKernel::ExecuteInstrumentedKernel()
	{
		name = "ExecuteInstrumentedKernel";
		
		description = "A utility to execute the instrumented PTX kernel generated by PTX Optimizer.";
	}
}

int main(int argc, char** argv)
{
	hydrazine::ArgumentParser parser( argc, argv );
	test::ExecuteInstrumentedKernel test;
	parser.description( test.testDescription() );

	parser.parse( "-s", "--seed", test.seed, 0, 
		"Random number generator seed, 0 implies seed with time." );
	parser.parse( "-v", "--verbose", test.verbose, false, 
		"Print out information after the test has finished." );
    parser.parse( "-i", "--input", test.input, "",
		"The instrumented ptx file." );
    parser.parse( "-m", "--module", test.moduleName, 
		"", "The name of the instrumented module." );
    parser.parse( "-k", "--kernel", test.kernelName, 
		"", "The name of the instrumented kernel." );
	parser.parse();

    if(test.input.empty()) {
        std::cout << "No ptx file provided for input. Bailing out.\n";
        return 0;
    }
	
    if(test.moduleName.empty()) {
        std::cout << "No module name provided. Bailing out.\n";
        return 0;
    }

    if(test.kernelName.empty()) {
        std::cout << "No kernel name provided. Bailing out.\n";
        return 0;
    }

	test.test();
	
	return test.passed();	
}

#endif

