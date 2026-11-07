#include <TApplication.h>
#include <TCanvas.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <TLatex.h>

#include "Garfield/AvalancheMC.hh"
#include "Garfield/ComponentAnsys123.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Random.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/ViewDrift.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/ViewField.hh"

using namespace Garfield;

int main(int argc, char* argv[]) {
  TApplication app("app", &argc, argv);

  // ====================================================================
  // Gas definition (unchanged)
  // ====================================================================
  MediumMagboltz gas("ar", 80., "co2", 20.);
  gas.SetTemperature(293.15);
  gas.SetPressure(900.);
  gas.Initialise(true);
  constexpr double rPenning = 0.51;
  constexpr double lambdaPenning = 0.;
  gas.EnablePenningTransfer(rPenning, lambdaPenning, "ar");
  gas.LoadIonMobility("IonMobility_Ar+_Ar.txt");
  std::string gasFile = "ar_80_dme_20_bis.gas";
  gas.LoadGasFile(gasFile.c_str());

  // ====================================================================
  // Load the DRIFT field map
  //
  // This corresponds to the APDL script that builds the full cylindrical
  // Ingrid / Cu-anode / drift-gap / Ti-cathode stack (diaAnodeCathode =
  // 28 mm). It is a real, finite volume -- NOT a periodic unit cell like
  // the GridPix hole field -- so we do NOT call EnableMirrorPeriodicityX/Y.
  //
  // ====================================================================
  ComponentAnsys123 fm;
  fm.Initialise("ELIST.lis", "NLIST.lis", "MPLIST.lis", "PRNSOL.lis", "mm");
  fm.PrintRange();
  fm.SetGas(&gas);
  fm.PrintMaterials();

  // ====================================================================
  // Geometric constants, copied from the APDL script (all cm)
  // ====================================================================

  const double marginfact = 0.03; // Fraction (30%) to extend the margin of plots

  constexpr double ingridThick      = 0.0010;
  constexpr double cuAnodeClearance = 0.09;
  constexpr double cuAnodeThick     = 0.05;
  constexpr double driftgap         = 1.0;
  constexpr double tiCathodeThick   = 0.10;
  constexpr double diaAnodeCathode  = 2.80;

  constexpr double h1 = 0.0;
  constexpr double h2 = h1 + ingridThick;   // top surface of the Ingrid
  constexpr double h3 = h2 + cuAnodeClearance;
  constexpr double h4 = h3 + cuAnodeThick;  // top of the Cu anode
  constexpr double h5 = h4 + driftgap;      // bottom of the cathode/window
  constexpr double h6 = h5 + tiCathodeThick;

  // Print z-heights of asic, anode and window
  std::cout << "ASIC+InGrid: " << h1 << "\n";
  std::cout << "Anode      : " << h3 << "\n";
  std::cout << "Window     : " << h5 << "\n";

  const double zGrid = h2;  // electrons are considered "collected" here
  const double rMax  = 0.5 * diaAnodeCathode + marginfact * diaAnodeCathode; // Half dia plus 30% margin
  // actually assigned in this geometry: Aluminum (Ingrid/Anode/Cathode)
  // and the gas, so the colour table is simpler than before.
  // ====================================================================
  ViewField fieldView(&fm);
  ViewFEMesh meshView(&fm);
  constexpr bool plotField = true;

  // ====================================================================
  // Drift Field visualization for the SPIE proceeding 2026
  // ====================================================================
  // if (plotField) {
  //   TCanvas* cf = new TCanvas("cf", "Drift-field visualizer", 600, 600);
  //   cf->SetLeftMargin(0.16);

  //   fieldView.SetCanvas(cf);
  //   fieldView.SetPlane(0, -1, 0, 0, 0, 0);
  //   // fieldView.SetArea(-rMax, h1 - 0.01, rMax, h6 + 0.03);
  //   fieldView.SetArea(-1, h1 - 0.01, 1, 1.05);
  //   fieldView.SetVoltageRange(-1600, 0.);
  //   fieldView.PlotContour("v");

  //   meshView.SetCanvas(cf);
  //   meshView.SetPlane(0, -1, 0, 0, 0, 0);
  //   meshView.SetArea(-rMax, h1 - 0.01, rMax, h6 + 0.01);
  //   meshView.SetFillMesh(true);
  //   meshView.SetColor(0, kRed);    // Material 1: Aluminum (Ingrid/Anode/Cathode)
  //   meshView.SetColor(1, kWhite);  // Material 2: Gas
  //   meshView.Plot(true);

  // }

  // ====================================================================
  // Electric Field Visualization for the JATIS paper
  // ====================================================================
  if (plotField) {
    TCanvas* cf = new TCanvas("cf", "Drift-field visualizer", 700, 600);
    cf->SetLeftMargin(0.16);
    cf->SetRightMargin(0.15);   // room for the colour bar
    cf->SetTopMargin(0.09);   // room above the plot for the label

    // One common area for all layers (x-z plane)
    const double xMin = -rMax, xMax = rMax;
    const double zMin = h1 - 0.01, zMax = h6 + 0.01;

    // Potential map
    fieldView.SetCanvas(cf);
    fieldView.SetPlane(0, -1, 0, 0, 0, 0);
    // fieldView.SetArea(xMin, zMin, xMax, zMax);
    fieldView.SetArea(-1.4, zMin, 1.4, zMax);
    fieldView.SetVoltageRange(-1600., 0.);
    fieldView.PlotContour("v");

    // Electrodes only (don't fill the gas, or it hides the contour)
    meshView.SetCanvas(cf);
    meshView.SetPlane(0, -1, 0, 0, 0, 0);
    // meshView.SetArea(xMin, zMin, xMax, zMax);
    meshView.SetArea(-1.4, zMin, 1.4, zMax);
    meshView.SetFillMesh(true);
    meshView.SetColor(0, kGray + 1);   // check index with PrintMaterials()
    meshView.DisableMaterial(1);       // gas: not drawn
    meshView.Plot(true);

    // Field lines, starting just below the cathode
    std::vector<double> xf, yf, zf;
    const double zStart = h5 - 0.001;  // 10 um below the cathode
    const double xEdge  = 0.5 * diaAnodeCathode - 0.01;
    fieldView.EqualFluxIntervals(-xEdge, 0., zStart, xEdge, 0., zStart,
                                 xf, yf, zf, 40);
    fieldView.PlotFieldLines(xf, yf, zf, true, false, kBlack);

    // Colour-bar label
    TLatex label;
    label.SetNDC();            // coordinates as fractions of the canvas
    label.SetTextAlign(22);    // centred
    label.SetTextSize(0.035);
    label.DrawLatex(0.88, 0.955, "Potential (V)");

    cf->Update();
    cf->SaveAs("driftfield.pdf");
  }

  app.Run();
  exit(0);

  // ====================================================================
  // Sensor
  // ====================================================================
  Sensor sensor(&fm);
  sensor.SetArea(-rMax, -rMax, h1 - 0.001, rMax, rMax, h6 + 0.001);

  AvalancheMC drift(&sensor);
  drift.SetDistanceSteps(2.e-4);

  ViewDrift driftView;
  constexpr bool plotDrift = true;
  if (plotDrift) drift.EnablePlotting(&driftView);

  // ====================================================================
  // 6x6 electron matrix, launched at a fixed height z0 in the drift gap.
  // ====================================================================

  // --- starting height --------------------------------------------------
  // must lie between h4 (top of Cu
  // anode) and h5 (bottom of cathode) to be inside the drift-gap volume.
  const double z0 = h5 - 0.5;
  
    // --- matrix geometry ----------------------------------------------------
  // 6x6 grid of (x0,y0) positions, evenly spaced from -7 mm to +7 mm.
  constexpr int nX = 6;
  constexpr int nY = 6;
  constexpr int nRepeats = 1000;  // electrons launched per (x0,y0) position
 
  const double pixelPitch = 0.0055;    // cm (55 um)
  const double matrixHalfRange = 0.55;  // cm (7 mm - 1.5 mm)
 
  std::vector<double> xStart, yStart;
  for (int i = 0; i < nX; ++i) {
    const double x0 = -matrixHalfRange +
                       (nX > 1 ? i * (2.0 * matrixHalfRange) / (nX - 1) : 0.0);
    for (int j = 0; j < nY; ++j) {
      const double y0 = -matrixHalfRange +
                         (nY > 1 ? j * (2.0 * matrixHalfRange) / (nY - 1) : 0.0);
      xStart.push_back(x0);
      yStart.push_back(y0);
    }
  }


    // --- manual binning of landing positions on the Ingrid, 55x55 um pixels.
  // histHalfRange must cover the full +/-7 mm launch matrix plus whatever
  // diffusion spreads it further -- widen if the CSV shows hits piling up
  // in the edge bins.
  const double histHalfRange = 1.0;  // cm (10 mm)
  const double histXMin = -histHalfRange;
  const double histYMin = -histHalfRange;
  const int nBinsX = static_cast<int>(2. * histHalfRange / pixelPitch);
  const int nBinsY = nBinsX;
  std::vector<std::vector<int>> hitCount(nBinsX, std::vector<int>(nBinsY, 0));
 
  // Raw per-electron start/end points (in mm), for anything you want to do
  // in Python beyond the binned histogram (e.g. drift-line spread, outliers).
  std::ofstream rawFile("electron_hits_raw.csv");
  rawFile << "position,repeat,x0_mm,y0_mm,z0_mm,x1_mm,y1_mm,z1_mm,reached_grid\n";
 
  std::size_t nStarted = 0;
  std::size_t nReachedGrid = 0;
 
  for (std::size_t p = 0; p < xStart.size(); ++p) {
    const double x0 = xStart[p];
    const double y0 = yStart[p];
    const double t0 = 0.;
 
    for (int r = 0; r < nRepeats; ++r) {
      std::cout << "Drifting electron, position " << p + 1 << "/" << xStart.size()
                << ", repeat " << r + 1 << "/" << nRepeats
                << ", from (x0,y0,z0) = (" << x0 * 10 << ", " << y0 * 10 << ", "
                << z0 * 10 << ") mm\n";
 
      drift.DriftElectron(x0, y0, z0, t0);
      ++nStarted;
 
      // NOTE: this assumes AvalancheMC::GetElectrons() returns objects with
      // a .path of points exposing .x/.y/.z/.t, mirroring how GetIons() is
      // used elsewhere in your codebase. If your Garfield++ build instead
      // exposes GetElectronEndpoint(i, x0,y0,z0,t0, x1,y1,z1,t1, status),
      // swap the two lines below for that call.
      const auto& electrons = drift.GetElectrons();
      if (electrons.empty() || electrons.front().path.empty()) continue;
 
      const auto& endpoint = electrons.front().path.back();
      const bool reachedGrid = endpoint.z <= zGrid + 0.001;
      if (reachedGrid) ++nReachedGrid;
 
      rawFile << p << "," << r << "," << x0 * 10 << "," << y0 * 10 << ","
              << z0 * 10 << "," << endpoint.x * 10 << "," << endpoint.y * 10 << ","
              << endpoint.z * 10 << "," << (reachedGrid ? 1 : 0) << "\n";
 
      // Bin the landing position, whether or not it strictly reached zGrid
      // (drop this check if you only want confirmed hits on the grid).
      if (!reachedGrid) continue;
      const int ix = static_cast<int>((endpoint.x - histXMin) / pixelPitch);
      const int iy = static_cast<int>((endpoint.y - histYMin) / pixelPitch);
      if (ix >= 0 && ix < nBinsX && iy >= 0 && iy < nBinsY) {
        ++hitCount[ix][iy];
      }
    }
  }
  rawFile.close();
 
  // Write the binned 55x55 um histogram to CSV: one row per non-empty bin.
  std::ofstream histFile("pixel_hit_histogram.csv");
  histFile << "ix,iy,x_center_mm,y_center_mm,count\n";
  for (int ix = 0; ix < nBinsX; ++ix) {
    for (int iy = 0; iy < nBinsY; ++iy) {
      if (hitCount[ix][iy] == 0) continue;
      const double xc = (histXMin + (ix + 0.5) * pixelPitch) * 10.0;
      const double yc = (histYMin + (iy + 0.5) * pixelPitch) * 10.0;
      histFile << ix << "," << iy << "," << xc << "," << yc << ","
                << hitCount[ix][iy] << "\n";
    }
  }
  histFile.close();
 
  std::cout << "\n==================================================\n";
  std::cout << "  DRIFT-FIELD ELECTRON COLLECTION SUMMARY\n";
  std::cout << "==================================================\n";
  std::cout << "Positions sampled:               " << xStart.size() << "\n";
  std::cout << "Repeats per position:            " << nRepeats << "\n";
  std::cout << "Electrons launched:              " << nStarted << "\n";
  std::cout << "Electrons reaching the Ingrid:   " << nReachedGrid << "\n";
  std::cout << "Wrote electron_hits_raw.csv and pixel_hit_histogram.csv\n";
  std::cout << "==================================================\n\n";
 
  // ====================================================================
  // Overlay drift lines on the mesh
  // ====================================================================
  if (plotDrift) {
    TCanvas* cd = new TCanvas("cd", "Electron drift lines", 800, 600);
    cd->SetLeftMargin(0.16);
    meshView.SetCanvas(cd);
    meshView.SetViewDrift(&driftView);
    meshView.SetPlane(0, -1, 0, 0, 0, 0);
    meshView.SetArea(-rMax, h1 - 0.01, rMax, h6 + 0.01);
    meshView.SetFillMesh(true);
    meshView.EnableAxes();
    meshView.SetColor(0, kRed);
    meshView.SetColor(1, kWhite);
    meshView.Plot(true, false);
  }


  app.Run();
}
