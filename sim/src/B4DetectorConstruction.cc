
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: B4DetectorConstruction.cc 101905 2016-12-07 11:34:39Z gunter $
// 
/// \file B4DetectorConstruction.cc
/// \brief Implementation of the B4DetectorConstruction class

#include "B4DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4Sphere.hh" // included by rp for sphere
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4Paraboloid.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
/*
   Flow of program



*/
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal 
G4GlobalMagFieldMessenger* B4DetectorConstruction::fMagFieldMessenger = nullptr; 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4DetectorConstruction::B4DetectorConstruction()
    : G4VUserDetectorConstruction(),
    fCheckOverlaps(true)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4DetectorConstruction::~B4DetectorConstruction()
{ 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B4DetectorConstruction::Construct()
{
    // Define materials 
    DefineMaterials();

    // Define volumes
    return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::DefineMaterials()
{ 
    // Lead material defined using NIST Manager
    auto nistManager = G4NistManager::Instance();
    nistManager->FindOrBuildMaterial("G4_AIR");

    // Liquid argon material
    G4double a;  // mass of a mole;
    G4double z;  // z=mean number of protons;  
    G4double density;
    G4int ncomponents, natoms; 
    G4Element* C = new G4Element("Carbon", "C", z=6., a=12.01*g/mole);
    G4Element* H = new G4Element("Hydrogen", "H", z=1., a=1.01*g/mole);
    new G4Material("liquidArgon", z=18., a= 39.95*g/mole, density= 1.390*g/cm3);
    // The argon by NIST Manager is a gas with a different density
    new G4Material("iron", z=26.,a=55.850*g/mole, density=7.894*g/cm3);
    new G4Material("tungsten", z=74.,a=183.85*g/mole, density=19.3*g/cm3);
    new G4Material("copper", z=29.,a=63.54*g/mole, density=8.96*g/cm3); 
    new G4Material("lead", z=82.,a=207.19*g/mole, density=11.34*g/cm3);
    // Vacuum
    new G4Material("Galactic", z=1., a=1.01*g/mole,density= universe_mean_density,
            kStateGas, 2.73*kelvin, 3.e-18*pascal);
    // Scintillator material
    G4Material* Scintillator = 
        new G4Material("Scintillator", density= 1.032*g/cm3, ncomponents=2);
    Scintillator->AddElement(C, natoms=9);
    Scintillator->AddElement(H, natoms=10);

    Scintillator->GetIonisation()->SetBirksConstant(0.126*mm/MeV);
    // Water
    G4Element* ele_H = new G4Element("Hydrogen","H",z=1.,a = 1.01*g/mole);
    G4Element* ele_O = new G4Element("Oxygen","O",z=8.,a=16.00*g/mole);
    G4Material* H2O = new G4Material("Water",density=1.000*g/cm3,ncomponents=2);
    H2O->AddElement(ele_H, natoms=2);
    H2O->AddElement(ele_O, natoms=1);

    // Print materials
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B4DetectorConstruction::DefineVolumes()
{
    /*  G4 Gerometry Tree
        World       (z: vertical (up: positive), x,y horizontal)
        - Tower
        - Tank LV(Tank1LV, Tank2LV, Tank3LV)
        - Wattank LV(WatTank1LV, WatTank2LV, WatTank3LV)
        - Tube LV(TubeLV)
        - Tube Water(TubeWaterLV)
        - ledge LV(LedgeLV)
        - leg LV(LegLV)
        - SC8     (xyz: similar to World)
        - Station    ( 1 copy for now) (xyz: similar to SC8)    
        - Tray    ( 4 copies)   (z: similar to Station, x along Bar's longer axis)
        - sBar (Scintillator Bar)  (10 copies)  (z same as world, x along longer axis)
        - Ref1    (reference plane)
	- PBbox
        */

    /// Geometry parameters ////////////////////////////////
    // Detector parameters 
    G4int nofBars = 10;
    G4double BarSizeX  = 60.*cm;
    G4double BarSizeY  = 5.*cm;
    G4double BarSizeZ  = 5.*cm;

    G4double TraySizeX  = BarSizeX;//+AirGap;
    G4double TraySizeY  = (BarSizeY/*+AirGap*/)*nofBars;//+AirGap;
    G4double TraySizeZ  = BarSizeZ;//+AirGap;
    G4double TrayPosX   = 0.0;
    G4double TrayPosY   = 0.0;
    G4double TrayPosZ[] = {593.73, 518.8, -518.8, -593.73};
    G4RotationMatrix* zRot = new G4RotationMatrix; // Rotates X and Z axes only
    zRot -> rotateX(0.*rad);
    zRot -> rotateY(0.*rad);
    zRot -> rotateZ(0.*rad);
    
    // Lead Box //////////////////////////////////////////
    G4double BoxSizeX = 1.*m;
    G4double BoxSizeY = 1.*m;
    G4double BoxSizeZ = 1.*m;

    /// Get materials /////////////////////////////////////////
    auto waterMaterial  = G4Material::GetMaterial("Water");
    auto tankMaterial  = G4Material::GetMaterial("iron");
    auto defaultMaterial = G4Material::GetMaterial("G4_AIR");
    //  auto waterMaterial = G4Material::GetMaterial("Water");
    // auto boxMaterial = G4Material::GetMaterial("iron");
    // auto boxMaterial = G4Material::GetMaterial("tungsten");
    //auto boxMaterial = G4Material::GetMaterial("copper");
    auto boxMaterial = G4Material::GetMaterial("lead");
    auto sBarMaterial  = G4Material::GetMaterial("Scintillator");
    if ( ! defaultMaterial || ! sBarMaterial ) {
        G4ExceptionDescription msg;
        msg << "Cannot retrieve materials already defined."; 
        G4Exception("B4DetectorConstruction::DefineVolumes()",
                "MyCode0001", FatalException, msg);
    }  

    //     
    // World
    //

    auto worldSizeX = 6000.0*cm ;  // half width
    auto worldSizeY = 3000.0*cm ;  // half width
    auto worldSizeZ = 6000.0*cm ;  // half width

    auto worldS 
        = new G4Box("World",           // its name
                worldSizeX, worldSizeY, worldSizeZ); // its size

    auto worldLV
        = new G4LogicalVolume(
                worldS,           // its solid
                defaultMaterial,  // its material
                "World");         // its name

    auto worldPV
        = new G4PVPlacement(
                0,                // no rotation
                G4ThreeVector(),  // at (0,0,0)
                worldLV,          // its logical volume                         
                "World",          // its name
                0,                // its mother  volume
                false,            // no boolean operation
                0,                // copy number
                fCheckOverlaps);  // checking overlaps 


    //                               
    // SC8
    //  

    auto SC8SizeX= 200.*cm;  // a half width
    auto SC8SizeY= 200.*cm;  // a half width
    auto SC8SizeZ= 1000.*cm; // a half width

    auto SC8S // creating solid for holding detector inside
        = new G4Box("SC8", SC8SizeX, SC8SizeY, SC8SizeZ);

    auto SC8LV // creating logical volume
        = new G4LogicalVolume(SC8S, defaultMaterial, "SC8");

    new G4PVPlacement(
            0,                // no rotation
            G4ThreeVector(),  // placement inside world volume
            SC8LV,          // its logical volume                         
            "SC8",    // its name
            worldLV,          // its mother  volume
            false,            // no boolean operation
            0,                // copy number
            fCheckOverlaps);  // checking overlaps 


    //                                 
    // Station
    //
    G4double StationSizeX  = 200.*cm;
    G4double StationSizeY  = 200.*cm;
    G4double StationSizeZ  = 150.*cm;

    G4RotationMatrix* stationRot = new G4RotationMatrix; // Rotates X and Z axes only
    //  std::cout<<"B4DetectorConstruction:  water tower, angle="<<angle<<std::endl;
    G4double angle= 45.0*M_PI/180.0;   // rotating detecor to face the water tower (45 degrees)
    //std::cout<<"B4DetectorConstruction:  angle="<<angle<<std::endl;
    stationRot -> rotateX(0.*rad);
    stationRot -> rotateY(0.*rad);
    stationRot -> rotateZ(0.*rad);

    auto Station1S
        = new G4Box("Station1",           // its name
                StationSizeX/2, StationSizeY/2, StationSizeZ/2); // its size

    auto Station1LV
        = new G4LogicalVolume(
                Station1S,           // its solid
                defaultMaterial,  // its material
                "Station1");         // its name
    new G4PVPlacement(
            stationRot,                // no rotation
            G4ThreeVector(0,0,0),  // at (0,0,0)
            Station1LV,          // its logical volume                         
            "Station1",    // its name
            SC8LV,          // its mother  volume
            false,            // no boolean operation
            0,                // copy number
            fCheckOverlaps);  // checking overlaps

    //   Reference Plane 1:  a thin horizontal plane at the center of station.

    auto RefPlane1S
        = new G4Box("RefPlane1",           // its name
                StationSizeX/2-1.0, StationSizeY/2-1.0, 1.0); // 
    auto RefPlane1LV
        = new G4LogicalVolume(
                RefPlane1S,        // its solid
                defaultMaterial, // its material
                "RefPlan1");          // its name

    new G4PVPlacement(
            0,                // no rotation
            G4ThreeVector(0.0, 0.0, 0.0),  // at (0,0,0)
            RefPlane1LV,          // its logical volume                         
            "RefPlane1",    // its name
            Station1LV,          // its mother  volume
            false,            // no boolean operation
            0,                // copy number
            fCheckOverlaps);  // checking overlaps

    //    Four trays, containing 10 sintillation bars...
    auto Tray1S
        = new G4Box("Tray1",           // its name
                TraySizeX/2, TraySizeY/2, TraySizeZ/2); // its size  
    auto Tray1LV
        = new G4LogicalVolume(
                Tray1S,        // its solid
                defaultMaterial, // its material
                "Tray1");          // its name

    new G4PVPlacement(              
            0,                // no rotation
            G4ThreeVector(TrayPosX,TrayPosY,TrayPosZ[0]),  // at (0,0,0)
            Tray1LV,          // its logical volume                         
            "Tray1",    // its name
            Station1LV,          // its mother  volume
            false,            // no boolean operation
            0,                // copy number
            fCheckOverlaps);  // checking overlaps

    new G4PVPlacement(
            zRot,                // no rotation
            G4ThreeVector(TrayPosX,TrayPosY,TrayPosZ[1]),  // at (0,0,0)
            Tray1LV,          // its logical volume                         
            "Tray1",    // its name
            Station1LV,          // its mother  volume
            false,            // no boolean operation
            1,                // copy number
            fCheckOverlaps);  // checking overlaps

    new G4PVPlacement(
            0,                // no rotation
            G4ThreeVector(TrayPosX,TrayPosY,TrayPosZ[2]),  // at (0,0,0)
            Tray1LV,          // its logical volume                         
            "Tray1",    // its name
            Station1LV,          // its mother  volume
            false,            // no boolean operation
            2,                // copy number
            fCheckOverlaps);  // checking overlaps

    new G4PVPlacement(
            zRot,                // no rotation
            G4ThreeVector(TrayPosX,TrayPosY,TrayPosZ[3]),  // at (0,0,0)
            Tray1LV,          // its logical volume                         
            "Tray1",    // its name
            Station1LV,          // its mother  volume
            false,            // no boolean operation
            3,                // copy number
            fCheckOverlaps);  // checking overlaps
    //std::cout<<"B4DetectorConstruction:  TrayPosZ2= "<<TrayPosZ[2]<<std::endl;
    //std::cout<<"B4DetectorConstruction:  TrayPosZ3= "<<TrayPosZ[3]<<std::endl;


    //
    // Lead Box
    //

    auto PBboxS
	= new G4Box("PBbox", //its name
		BoxSizeX, BoxSizeY, BoxSizeZ); // its size
    auto PBboxLV
	= new G4LogicalVolume(
	        PBboxS,
		boxMaterial,
		"PBbox");

    new G4PVPlacement(
	    0,
	    G4ThreeVector(0.0, 0.0, -worldSizeZ/2.0 ),
	    PBboxLV,
	    "PBbox",
	    worldLV,
	    false,
	    0,
	    fCheckOverlaps);


    //                               
    // Individual bar
    //
    auto sBARS
        = new G4Box("sBAR",             // its name
                BarSizeX/2.0, BarSizeY/2.0, BarSizeZ/2.0); // its size

    auto sBARLV
        = new G4LogicalVolume(
                sBARS,             // its solid
                sBarMaterial,      // its material
                "sBAR");           // its name

    for (int i=0; i<nofBars; i++) {
        double yval=-TraySizeY/2+BarSizeY/2.0/*+AirGap*/+(BarSizeY/*+AirGap*/)*float(i);
        // std::cout<<"  i  "<<i<<" yval "<<yval<<std::endl;                    
        new G4PVPlacement(
                0,                // no rotation
                G4ThreeVector(0.0,yval,0.0), // its position
                sBARLV,            // its logical volume                         
                "sBAR",            // its name
                Tray1LV,          // its mother  volume
                false,            // no boolean operation
                i,                // copy number
                fCheckOverlaps);  // checking overlaps 
    }
    std::cout<<"B4DetectorCOnstruction: nofBars="<<nofBars<<std::endl;
    //
    // print parameters
    //
    G4cout
        << G4endl 
        << "------------------------------------------------------------" << G4endl
        << "---> The calorimeter is " << nofBars << " bars of: [ "
        << BarSizeX/cm << "mm of " << sBarMaterial->GetName() << " ] " << G4endl
        << "------------------------------------------------------------" << G4endl;

    //                                       
    // Visualization attributes
    //

    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    // tank=green, waterinside=blue, airinside=red
    //
    worldLV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.0,0.0,1.0))); // blue
    SC8LV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.705882, 0.682353, 0.658824)));
    Station1LV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.0,1.0,0.0)));
    Tray1LV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.0,0.0,0.0)));
    sBARLV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.345098, 0.407843, 0.121569, 0.30)));
    PBboxLV->SetVisAttributes(new G4VisAttributes(TRUE,G4Colour(0.345098, 0.407843, 0.121569, 0.30)));

    //
    // Always return the physical World
    //
    return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4DetectorConstruction::ConstructSDandField()
{ 
    // Create global magnetic field messenger.
    // Uniform magnetic field is then created automatically if
    // the field value is not zero.
    G4ThreeVector fieldValue;
    fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
    fMagFieldMessenger->SetVerboseLevel(1);

    // Register the field messenger for deleting
    G4AutoDelete::Register(fMagFieldMessenger);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
