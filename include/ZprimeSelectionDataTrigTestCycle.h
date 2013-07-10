// Dear emacs, this is -*- c++ -*-
#ifndef ZprimeSelectionDataTrigTestCycle_H
#define ZprimeSelectionDataTrigTestCycle_H

// SFrame include(s):
#include "include/AnalysisCycle.h"
#include "Cleaner.h"
#include "HypothesisDiscriminator.h"
#include "ElectronHists.h"
#include "MuonHists.h"
#include "MuonEffHists.h"
#include "ElectronEffHists.h"
#include "JetHists.h"
#include "include/SelectionModules.h"

class ZprimeSelectionDataTrigTestCycle : public AnalysisCycle {

public:
  /// Default constructor
  ZprimeSelectionDataTrigTestCycle();
  /// Default destructor
  ~ZprimeSelectionDataTrigTestCycle();

  /// Function called at the beginning of the cycle
  void BeginCycle() throw( SError );
  /// Function called at the end of the cycle
  void EndCycle() throw( SError );

  /// Function called at the beginning of a new input data
  void BeginInputData( const SInputData& ) throw( SError );
  /// Function called after finishing to process an input data
  void EndInputData  ( const SInputData& ) throw( SError );

  /// Function called after opening each new input file
  void BeginInputFile( const SInputData& ) throw( SError );

  /// Function called for every event
  void ExecuteEvent( const SInputData&, Double_t ) throw( SError );

private:
  //
  // Put all your private variables here
  //
  
  std::string m_Electron_Or_Muon_Selection;
  int m_Nbtags_max;
  int m_Nbtags_min;  
  bool doEle;
  bool doMu;
  Cleaner* m_cleaner;
  Chi2Discriminator* m_chi2discr;
  BestPossibleDiscriminator* m_bpdiscr;

  // Macro adding the functions for dictionary generation
  ClassDef( ZprimeSelectionDataTrigTestCycle, 0 );

}; // class ZprimeSelectionDataTrigTestCycle

#endif // ZprimeSelectionDataTrigTestCycle_H

