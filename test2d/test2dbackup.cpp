#include "olb2D.h"
#include "olb2D.hh"   // use only generic version!
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;
using namespace std;

typedef double T;

const T lx  = 2.;             // length of the channel
const T ly  = 1.;             // height of the channel
int N = 50;                   // resolution of the model
const T Re = 10.;    
const T maxPhysT = 20.;       // max. simulation time in s, SI unit

T n = 1;              // parameter in power law model (n=1 Newtonian fluid)

#define DESCRIPTOR MRTD2Q9Descriptor
//#define DYNAMICS SmagorinskyPowerLawBGKdynamics
#define DYNAMICS MRTdynamics
//#define DESCRIPTOR DynOmegaD2Q9Descriptor


// Stores geometry information in form of material numbers
void prepareGeometry( UnitConverter<T,DESCRIPTOR> const& converter,
                      SuperGeometry2D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  superGeometry.rename( 0,2 );

  superGeometry.rename( 2,1,1,1 );
    Vector<T,2> extend;
    Vector<T,2> origin;
    T physSpacing = converter.getPhysDeltaX();

    // Set material number for inflow
    extend[1] = ly;
    extend[0] = physSpacing / 2;
    origin[0] -= physSpacing / 4;
    IndicatorCuboid2D<T> inflow( extend, origin );
    superGeometry.rename( 2,3,1,inflow );

    // Set material number for outflow
    origin[0] = lx - physSpacing / 4;
    IndicatorCuboid2D<T> outflow( extend, origin );
    superGeometry.rename( 2,4,1,outflow );
  // Removes all not needed boundary voxels outside the surface
  superGeometry.clean();
  // Removes all not needed boundary voxels inside the surface
  superGeometry.innerClean();
  superGeometry.checkForErrors();

  superGeometry.print();

  clout << "Prepare Geometry ... OK" << std::endl;
}

void powprepareGeometry( PowerLawUnitConverter<T,DESCRIPTOR> const& converter,
                      SuperGeometry2D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  superGeometry.rename( 0,2 );

  superGeometry.rename( 2,1,1,1 );
    Vector<T,2> extend;
    Vector<T,2> origin;
    T physSpacing = converter.getPhysDeltaX();

    // Set material number for inflow
    extend[1] = ly;
    extend[0] = physSpacing / 2;
    origin[0] -= physSpacing / 4;
    IndicatorCuboid2D<T> inflow( extend, origin );
    superGeometry.rename( 2,3,1,inflow );

    // Set material number for outflow
    origin[0] = lx - physSpacing / 4;
    IndicatorCuboid2D<T> outflow( extend, origin );
    superGeometry.rename( 2,4,1,outflow );
  // Removes all not needed boundary voxels outside the surface
  superGeometry.clean();
  // Removes all not needed boundary voxels inside the surface
  superGeometry.innerClean();
  superGeometry.checkForErrors();

  superGeometry.print();

  clout << "Prepare Geometry ... OK" << std::endl;
}

// Set up the geometry of the simulation
void prepareLattice( UnitConverter<T,DESCRIPTOR> const& converter,
                     SuperLattice2D<T, DESCRIPTOR>& sLattice,
                     Dynamics<T, DESCRIPTOR>& bulkDynamics,
                     sOnLatticeBoundaryCondition2D<T,DESCRIPTOR>& sBoundaryCondition,
                     SuperGeometry2D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareLattice" );
  clout << "Prepare Lattice ..." << std::endl;

  const T omega = converter.getLatticeRelaxationFrequency();

  // Material=0 -->do nothing
  sLattice.defineDynamics( superGeometry, 0, &instances::getNoDynamics<T, DESCRIPTOR>() );

  // Material=1 -->bulk dynamics
  sLattice.defineDynamics( superGeometry, 1, &bulkDynamics );

  sLattice.defineDynamics( superGeometry, 2, &instances::getBounceBack<T, DESCRIPTOR>() );


    // Material=3 -->bulk dynamics
    sLattice.defineDynamics( superGeometry, 3, &bulkDynamics );

    // Material=4 -->bulk dynamics
    sLattice.defineDynamics( superGeometry, 4, &bulkDynamics );

    sBoundaryCondition.addVelocityBoundary( superGeometry, 3, omega );
    sBoundaryCondition.addPressureBoundary( superGeometry, 4, omega );
  

  // Initial conditions
  T Lx = converter.getLatticeLength( lx );
  T Ly = converter.getLatticeLength( ly );

    T p0 =8.*converter.getLatticeViscosity()*converter.getCharLatticeVelocity()*Lx/( Ly*Ly );
    AnalyticalLinear2D<T,T> rho( -p0/lx*invCs2<T,DESCRIPTOR>(), 0 , p0*invCs2<T,DESCRIPTOR>()+1 );

    T maxVelocity = converter.getCharLatticeVelocity();
    T distance2Wall = converter.getConversionFactorLength();
    Poiseuille2D<T> u( superGeometry, 3, maxVelocity, distance2Wall );

    // Initialize all values of distribution functions to their local equilibrium
    sLattice.defineRhoU( superGeometry, 1, rho, u );
    sLattice.iniEquilibrium( superGeometry, 1, rho, u );
    sLattice.defineRhoU( superGeometry, 2, rho, u );
    sLattice.iniEquilibrium( superGeometry, 2, rho, u );
    sLattice.defineRhoU( superGeometry, 3, rho, u );
    sLattice.iniEquilibrium( superGeometry, 3, rho, u );
    sLattice.defineRhoU( superGeometry, 4, rho, u );
    sLattice.iniEquilibrium( superGeometry, 4, rho, u );
  

  // Make the lattice ready for simulation
  sLattice.initialize();

  clout << "Prepare Lattice ... OK" << std::endl;
}

// Set up the geometry of the simulation
void powprepareLattice( PowerLawUnitConverter<T,DESCRIPTOR> const& converter,
                     SuperLattice2D<T, DESCRIPTOR>& sLattice,
                     Dynamics<T, DESCRIPTOR>& bulkDynamics,
                     sOnLatticeBoundaryCondition2D<T,DESCRIPTOR>& sBoundaryCondition,
                     SuperGeometry2D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareLattice" );
  clout << "Prepare Lattice ..." << std::endl;

  const T omega = converter.getLatticeRelaxationFrequency();

  // Material=0 -->do nothing
  sLattice.defineDynamics( superGeometry, 0, &instances::getNoDynamics<T, DESCRIPTOR>() );

  // Material=1 -->bulk dynamics
  sLattice.defineDynamics( superGeometry, 1, &bulkDynamics );

  sLattice.defineDynamics( superGeometry, 2, &instances::getBounceBack<T, DESCRIPTOR>() );


    // Material=3 -->bulk dynamics
    sLattice.defineDynamics( superGeometry, 3, &bulkDynamics );

    // Material=4 -->bulk dynamics
    sLattice.defineDynamics( superGeometry, 4, &bulkDynamics );

    sBoundaryCondition.addVelocityBoundary( superGeometry, 3, omega );
    sBoundaryCondition.addPressureBoundary( superGeometry, 4, omega );
  

  // Initial conditions
  T Lx = converter.getLatticeLength( lx );
  T Ly = converter.getLatticeLength( ly );

    T p0 =8.*converter.getLatticeViscosity()*converter.getCharLatticeVelocity()*Lx/( Ly*Ly );
    AnalyticalLinear2D<T,T> rho( -p0/lx*invCs2<T,DESCRIPTOR>(), 0 , p0*invCs2<T,DESCRIPTOR>()+1 );

    T maxVelocity = converter.getCharLatticeVelocity();
    T distance2Wall = converter.getConversionFactorLength();
    Poiseuille2D<T> u( superGeometry, 3, maxVelocity, distance2Wall );

    // Initialize all values of distribution functions to their local equilibrium
    sLattice.defineRhoU( superGeometry, 1, rho, u );
    sLattice.iniEquilibrium( superGeometry, 1, rho, u );
    sLattice.defineRhoU( superGeometry, 2, rho, u );
    sLattice.iniEquilibrium( superGeometry, 2, rho, u );
    sLattice.defineRhoU( superGeometry, 3, rho, u );
    sLattice.iniEquilibrium( superGeometry, 3, rho, u );
    sLattice.defineRhoU( superGeometry, 4, rho, u );
    sLattice.iniEquilibrium( superGeometry, 4, rho, u );
  

  // Make the lattice ready for simulation
  sLattice.initialize();

  clout << "Prepare Lattice ... OK" << std::endl;
}


// Output to console and files
void getResults( SuperLattice2D<T,DESCRIPTOR>& sLattice, Dynamics<T, DESCRIPTOR>& bulkDynamics,
                 UnitConverter<T,DESCRIPTOR> const& converter, int iT,
                 SuperGeometry2D<T>& superGeometry, Timer<T>& timer ) {

  OstreamManager clout( std::cout,"getResults" );

  SuperVTMwriter2D<T> vtmWriter( "forcedPoiseuille2d" );
  SuperLatticePhysVelocity2D<T, DESCRIPTOR> velocity( sLattice, converter );
  SuperLatticePhysPressure2D<T, DESCRIPTOR> pressure( sLattice, converter );
  vtmWriter.addFunctor( velocity );
  vtmWriter.addFunctor( pressure );

  const int vtmIter  = converter.getLatticeTime( maxPhysT/20. );
  const int statIter = converter.getLatticeTime( maxPhysT/20. );

  if ( iT==0 ) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeGeometry2D<T, DESCRIPTOR> geometry( sLattice, superGeometry );
    SuperLatticeCuboid2D<T, DESCRIPTOR> cuboid( sLattice );
    SuperLatticeRank2D<T, DESCRIPTOR> rank( sLattice );
    superGeometry.rename( 0,2 );
    vtmWriter.write( geometry );
    vtmWriter.write( cuboid );
    vtmWriter.write( rank );

    vtmWriter.createMasterFile();
  }

  // Writes the vtm files and profile text file
  if ( iT%vtmIter==0 ) {
    vtmWriter.write( iT );

    SuperEuklidNorm2D<T, DESCRIPTOR> normVel( velocity );
    BlockReduction2D2D<T> planeReduction( normVel, 600, BlockDataSyncMode::ReduceOnly );
    // write output as JPEG
    heatmap::write(planeReduction, iT);
  }


  // Writes output on the console
  if ( iT%statIter==0 ) {
    // Timer console output
    timer.update( iT );
    timer.printStep();

    // Lattice statistics console output
    sLattice.getStatistics().print( iT,converter.getPhysTime( iT ) );

  }
}


// Output to console and files
void powgetResults( SuperLattice2D<T,DESCRIPTOR>& sLattice, Dynamics<T, DESCRIPTOR>& bulkDynamics,
                 PowerLawUnitConverter<T,DESCRIPTOR> const& converter, int iT,
                 SuperGeometry2D<T>& superGeometry, Timer<T>& timer ) {

  OstreamManager clout( std::cout,"powgetResults" );

  SuperVTMwriter2D<T> vtmWriter( "powforcedPoiseuille2d" );
  SuperLatticePhysVelocity2D<T, DESCRIPTOR> velocity( sLattice, converter );
  SuperLatticePhysPressure2D<T, DESCRIPTOR> pressure( sLattice, converter );
  vtmWriter.addFunctor( velocity );
  vtmWriter.addFunctor( pressure );

  const int vtmIter  = converter.getLatticeTime( maxPhysT/20. );
  const int statIter = converter.getLatticeTime( maxPhysT/20. );

  if ( iT==0 ) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeGeometry2D<T, DESCRIPTOR> geometry( sLattice, superGeometry );
    SuperLatticeCuboid2D<T, DESCRIPTOR> cuboid( sLattice );
    SuperLatticeRank2D<T, DESCRIPTOR> rank( sLattice );
    superGeometry.rename( 0,2 );
    vtmWriter.write( geometry );
    vtmWriter.write( cuboid );
    vtmWriter.write( rank );

    vtmWriter.createMasterFile();
  }

  // Writes the vtm files and profile text file
  if ( iT%vtmIter==0 ) {
    vtmWriter.write( iT );

    SuperEuklidNorm2D<T, DESCRIPTOR> normVel( velocity );
    BlockReduction2D2D<T> planeReduction( normVel, 600, BlockDataSyncMode::ReduceOnly );
    // write output as JPEG
    heatmap::write(planeReduction, iT);
  }


  // Writes output on the console
  if ( iT%statIter==0 ) {
    // Timer console output
    timer.update( iT );
    timer.printStep();

    // Lattice statistics console output
    sLattice.getStatistics().print( iT,converter.getPhysTime( iT ) );

  }
}




int main( int argc, char* argv[] ) {

    printf("\n\n starting...\n");
      // === 1st Step: Initialization ===
  olbInit( &argc, &argv );
  singleton::directories().setOutputDir( "./tmp/" );
  OstreamManager clout( std::cout,"main" );

      UnitConverterFromResolutionAndRelaxationTime<T, DESCRIPTOR> const converter(
    int {N},     // resolution: number of voxels per charPhysL
    (T)   0.8,   // latticeRelaxationTime: relaxation time, have to be greater than 0.5!
    (T)   1,     // charPhysLength: reference length of simulation geometry
    (T)   1,     // charPhysVelocity: maximal/highest expected velocity during simulation in __m / s__
    (T)   1./Re, // physViscosity: physical kinematic viscosity in __m^2 / s__
    (T)   1.0    // physDensity: physical density in __kg / m^3__
  );
  // Prints the converter log as console output
  converter.print();
  // Writes the converter log in a file
  converter.write("test2d");
  
    PowerLawUnitConverterFrom_Resolution_RelaxationTime_Reynolds_PLindex<T, DESCRIPTOR> const powconverter(
  int {N},     // resolution: number of voxels per charPhysL
  (T)   0.8,   // latticeRelaxationTime: relaxation time, have to be greater than 0.5!
  (T)   1,     // charPhysLength: reference length of simulation geometry
  (T)   1,     // charPhysVelocity: maximal/highest expected velocity during simulation in __m / s__
  (T)   Re,        // Reynolds number
  (T)   n,     // power-law index
  (T)   1.0    // physDensity: physical density in __kg / m^3__
  );
  // Prints the converter log as console output
  powconverter.print();
  // Writes the converter log in a file
  powconverter.write("powerLaw2d");
  
    // === 2nd Step: Prepare Geometry ===
  Vector<T,2> extend( lx, ly );
  Vector<T,2> origin;
  IndicatorCuboid2D<T> cuboid( extend, origin );
  const int noOfCuboids = 1;
  CuboidGeometry2D<T> cuboidGeometry( cuboid, converter.getConversionFactorLength(), noOfCuboids );
    // Instantiation of a loadBalancer
  HeuristicLoadBalancer<T> loadBalancer( cuboidGeometry );

  // Instantiation of a superGeometry
  SuperGeometry2D<T> superGeometry( cuboidGeometry, loadBalancer, 2 );
  SuperGeometry2D<T> powsuperGeometry( cuboidGeometry, loadBalancer, 2 );
  SuperGeometry2D<T> porouspowsuperGeometry( cuboidGeometry, loadBalancer, 2 );
  
  prepareGeometry( converter, superGeometry );
  powprepareGeometry( powconverter, powsuperGeometry );
  powprepareGeometry( powconverter, porouspowsuperGeometry );
  
  
      // === 3rd Step: Prepare Lattice ===
  SuperLattice2D<T, DESCRIPTOR> sLattice( superGeometry );
  SuperLattice2D<T, DESCRIPTOR> powsLattice( powsuperGeometry );
  SuperLattice2D<T, DESCRIPTOR> porouspowsLattice( porouspowsuperGeometry );

  //Dynamics<T, DESCRIPTOR>* bulkDynamics;

  //bulkDynamics = new MRTdynamics<T, DESCRIPTOR>( converter.getLatticeRelaxationFrequency(), instances::getBulkMomenta<T, DESCRIPTOR>() );
  MRTdynamics<T, DESCRIPTOR> bulkDynamics ( converter.getLatticeRelaxationFrequency(), instances::getBulkMomenta<T, DESCRIPTOR>() );
                //power law dynamics
                T distance2Wall = powconverter.getConversionFactorLength()/2.;
                T p0 = powconverter.getPhysConsistencyCoeff()*pow( powconverter.getCharPhysVelocity(),n )*pow( ( n + 1. )/n,n )*pow( 2./( ly-distance2Wall*2 ),n + 1. );

                clout << "Dimensionalized version-1." << std::endl;
                PowerLawBGKdynamics<T, DESCRIPTOR> powbulkDynamics( powconverter.getLatticeRelaxationFrequency(), instances::getBulkMomenta<T, DESCRIPTOR>(), powconverter.getLatticeConsistencyCoeff(), n );
                
                //Dynamics<T,DESCRIPTOR>* porouspowBulkDynamics;
                //porouspowBulkDynamics = new SmagorinskyPowerLawBGKdynamics<T, DESCRIPTOR>( powconverter.getLatticeRelaxationFrequency(), instances::getBulkMomenta<T, DESCRIPTOR>(), powconverter.getLatticeConsistencyCoeff(), n ,0.1);
                SmagorinskyPowerLawBGKdynamics<T, DESCRIPTOR> porouspowBulkDynamics( powconverter.getLatticeRelaxationFrequency(), instances::getBulkMomenta<T, DESCRIPTOR>(), powconverter.getLatticeConsistencyCoeff(), n ,0.1);
      
                
      // choose between local and non-local boundary condition
  sOnLatticeBoundaryCondition2D<T, DESCRIPTOR> sBoundaryCondition( sLattice );
  createLocalBoundaryCondition2D<T, DESCRIPTOR> (sBoundaryCondition);

  sOnLatticeBoundaryCondition2D<T, DESCRIPTOR> powsBoundaryCondition( powsLattice );
  createLocalBoundaryCondition2D<T, DESCRIPTOR> (powsBoundaryCondition);
  
 sOnLatticeBoundaryCondition2D<T, DESCRIPTOR> porouspowsBoundaryCondition( porouspowsLattice );
 createLocalBoundaryCondition2D<T, DESCRIPTOR> (porouspowsBoundaryCondition);
  
    prepareLattice( converter, sLattice, bulkDynamics, sBoundaryCondition, superGeometry );
    powprepareLattice( powconverter, powsLattice, powbulkDynamics, powsBoundaryCondition, powsuperGeometry );
    powprepareLattice( powconverter, porouspowsLattice, porouspowBulkDynamics, porouspowsBoundaryCondition, porouspowsuperGeometry );

    
  Timer<T> timer( converter.getLatticeTime( maxPhysT ), superGeometry.getStatistics().getNvoxel() );
  timer.start();
          clout << "starting simulation..." << endl;
     for ( int iT = 0; iT < converter.getLatticeTime( maxPhysT ); ++iT ) {
    clout << "Iterating step" << iT << endl;
    // === 6th Step: Collide and Stream Execution ===
    sLattice.collideAndStream();
    //getResults( sLattice, *bulkDynamics, converter, iT, superGeometry, timer );

    porouspowsLattice.collideAndStream();
    //powgetResults( porouspowsLattice, porouspowBulkDynamics, powconverter, iT, porouspowsuperGeometry, timer );
    //powsLattice.collideAndStream();
  }

  timer.stop();
  timer.printSummary();
}
