/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: vtkMEDFillingHoleTest.h,v $
Language:  C++
Date:      $Date: 2010-06-14 12:39:04 $
Version:   $Revision: 1.1.2.2 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __CPP_UNIT_vtkMEDFillingHoleTEST_H__
#define __CPP_UNIT_vtkMEDFillingHoleTEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>



class vtkMEDFillingHoleTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( vtkMEDFillingHoleTest );
  CPPUNIT_TEST( TestDynamicAllocation );
  CPPUNIT_TEST( TestSetFillAHole );
  CPPUNIT_TEST( TestSetFillAllHole );
  CPPUNIT_TEST( TestGetLastPatch );
  CPPUNIT_TEST_SUITE_END();

protected:
  void TestDynamicAllocation();
  void TestSetFillAHole();
  void TestSetFillAllHole();
  void TestGetLastPatch();
};


int
main( int argc, char* argv[] )
{
  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that colllects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  CPPUNIT_NS::TestRunner runner;
  runner.addTest( vtkMEDFillingHoleTest::suite());
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
  outputter.write(); 

  return result.wasSuccessful() ? 0 : 1;
}

#endif