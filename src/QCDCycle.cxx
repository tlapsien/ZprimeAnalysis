// $Id: QCDCycle.cxx,v 1.0 2012/11/09 10:06:24 gonvaq Exp $

#include <iostream>

using namespace std;

// Local include(s):
#include "include/JetEffiHists.h"
#include "include/QCDCycle.h"
#include "include/SelectionModules.h"
#include "include/ExampleHists.h"
#include "include/HypothesisHists.h"
#include "include/JetHists.h"
#include "include/MuonHists.h"
#include "include/TopJetHists.h"
#include "include/EventHists.h"
#include "include/TopEffiHists.h"
#include "include/MJetsHists.h"
#include "include/NeutrinoHists.h"
#include "include/TopFitCalc.h"
#include "include/DelRHists.h"
#include "include/TopTagDelRHists.h"
#include "include/RazorHists.h"
#include "include/NeutrinoHists.h"
#include "include/ConstituentsHists.h"
#include "include/MetaTreeFiller.h"
#include "include/CleanerFiller.h"

ClassImp( QCDCycle );

QCDCycle::QCDCycle()
   : AnalysisCycle() {

  // constructor, declare additional variables that should be 
  // obtained from the steering-xml file
  
  // set the integrated luminosity per bin for the lumi-yield control plots
  SetIntLumiPerBin(500.);

  m_mttgencut = false;
  DeclareProperty( "ApplyMttbarGenCut", m_mttgencut );
  m_jes_unc = NULL;
  m_dobsf = "none";

  m_tree=NULL;

  m_btagtype = e_CSVT; 

  //default: no btagging cuts applied, other cuts can be defined in config file
  m_Nbtags_min=0;
  m_Nbtags_max=int_infinity();
  DeclareProperty( "Nbtags_min", m_Nbtags_min);
  DeclareProperty( "Nbtags_max", m_Nbtags_max);

  DeclareProperty( "Electron_Or_Muon_Selection", m_Electron_Or_Muon_Selection );
  DeclareProperty( "BTaggingScaleFactors", m_dobsf );


}

QCDCycle::~QCDCycle()
{
  // destructor
  if (m_jes_unc) delete m_jes_unc;
  

}

void QCDCycle::BeginCycle() throw( SError ) 
{
  // Start of the job, general set-up and definition of 
  // objects are done here

  // Important: first call BeginCycle of base class
  AnalysisCycle::BeginCycle();

  return;

}

void QCDCycle::EndCycle() throw( SError ) 
{
  // clean-up, info messages and final calculations after the analysis

  
  // call the base cycle class for all standard methods
  AnalysisCycle::EndCycle();

  return;

}

void QCDCycle::BeginInputData( const SInputData& id ) throw( SError ) 
{

   // declaration of histograms and selections

  // Important: first call BeginInputData of base class
  AnalysisCycle::BeginInputData( id );

  // -------------------- set up the selections ---------------------------

  bool doEle=false;
  bool doMu=false;
  
  if(m_Electron_Or_Muon_Selection=="Electrons" || m_Electron_Or_Muon_Selection=="Electron" || m_Electron_Or_Muon_Selection=="Ele" || m_Electron_Or_Muon_Selection=="ELE") {
    doEle=true;
  } else if(m_Electron_Or_Muon_Selection=="Muon" || m_Electron_Or_Muon_Selection=="Muons" || m_Electron_Or_Muon_Selection=="Mu" || m_Electron_Or_Muon_Selection=="MU") {
    doMu=true;
  } else {
    m_logger << ERROR << "Electron_Or_Muon_Selection is not defined in your xml config file --- should be either `ELE` or `MU`" << SLogger::endmsg;
  }

 
  m_tree = GetOutputMetadataTree("CutTree");

  // event filter for HCAL laser events
  Selection* HCALlaser = new Selection("HCAL_laser_events");
  HCALlaser->addSelectionModule(new EventFilterSelection(m_filter_file.c_str()) );



  m_chi2discr = new Chi2Discriminator();
  m_tagchi2discr = new Chi2Discriminator();

  Selection* BSel = new Selection( "BSelection");
  BSel->addSelectionModule(new NBTagSelection(1,int_infinity(),m_btagtype)); //at least one b tag

  Selection* NoBSel = new Selection( "NoBSelection");
  NoBSel->addSelectionModule(new NBTagSelection(0,0,m_btagtype)); //no b tags

  Selection* MuonSel = new Selection("MuonSelection");
  MuonSel->addSelectionModule(new NMuonSelection (1, 1));

  Selection* NJetSel = new Selection("NJetSelection");
  NJetSel->addSelectionModule(new NJetSelection(2,int_infinity(),50,2.5));
  NJetSel->addSelectionModule(new NJetSelection(1,int_infinity(),210,2.5));

  Selection* TwoDMuonSel = new Selection("TwoDSelection");
  TwoDMuonSel->addSelectionModule(new TwoDCutMuon(.5,25));

  Selection* IsoConeSel = new Selection("IsoConeSelection");
  IsoConeSel->addSelectionModule(new IsoConeSelection());


  static Selection* mttbar_gen_selection = new Selection("Mttbar_Gen_Selection");
  
  if ((id.GetVersion() == "TTbar")&& m_mttgencut){
    m_logger << INFO << "Applying mttbar generator cut from 700 to inf GeV." << SLogger::endmsg;
    mttbar_gen_selection->addSelectionModule(new MttbarGenCut(700));
    mttbar_gen_selection->EnableSelection();
    }
  else {
    m_logger << INFO << "Disabling mttbar generator cut." << SLogger::endmsg;
    mttbar_gen_selection->DisableSelection();
  }
  /*
  if ((id.GetVersion() == "TTbar")||(id.GetVersion() == "TT_scaleup")|| (id.GetVersion() == "TT_scaledown")){
    m_logger << INFO << "Applying mttbar generator cut from 0 to 700 GeV." << SLogger::endmsg;
    mttbar_gen_selection->addSelectionModule(new MttbarGenCut(0,700));
    mttbar_gen_selection->EnableSelection();
  }
  else {
    m_logger << INFO << "Disabling mttbar generator cut." << SLogger::endmsg;
    mttbar_gen_selection->DisableSelection();
  }
  */

  //Selection* TopSel = new Selection("TopSelection");
  //TopSel->addSelectionModule(new NJetSelection (1, int_infinity(), 150., 2.5));
  //TopSel->addSelectionModule(new NJetSelection (2, int_infinity(), 50.));
  //TopSel->addSelectionModule(new METCut(50));
  //TopSel->addSelectionModule(new HTlepCut(150));


  Selection* PTSel = new Selection("PTSelection");
  PTSel->addSelectionModule(new NJetSelection (1, int_infinity(), 230., 2.5));

  Selection* METSel = new Selection("METSelelection");
  METSel->addSelectionModule(new METCut(50));

  Selection* HTmuonSel = new Selection("HTmuonSelelection");
  HTmuonSel->addSelectionModule(new HTmuonCut(130));


  //TopSel->addSelectionModule(new NBTagSelection(m_Nbtags_min,m_Nbtags_max));
  //TopSel->addSelectionModule(new METCut(20));
  //TopSel->addSelectionModule(new TriggerSelection("HLT_PFJet320_v"));
  //TopSel->addSelectionModule(new NTopJetSelection(1,int_infinity(),0,2.5));
  //TopSel->addSelectionModule(new CAAntiktJetSelection(1,2,1,int_infinity(),int_infinity()));
  //TopSel->addSelectionModule(new TopTagAntiktJetSelection(1,2,0.8,int_infinity(),int_infinity()));


  Selection* RazorSel = new Selection("RazorSelection");
  RazorSel->addSelectionModule(new RazorSelection(m_tagchi2discr));


  Selection* TopTagSel = new Selection("TopTagSelection");
  TopTagSel->addSelectionModule(new NTopTagSelection(1));
  TopTagSel->addSelectionModule(new TopTagOverlapSelection());

  Selection* Chi2Seletion = new Selection("Chi2Selection");
  Chi2Seletion->addSelectionModule(new HypothesisDiscriminatorCut(m_chi2discr, -1*double_infinity(), 50));

  Selection* Chi2NoTagSeletion = new Selection("Chi2NoTagSelection");
  Chi2NoTagSeletion->addSelectionModule(new HypothesisDiscriminatorCut(m_chi2discr, -1*double_infinity(), 30));


  RegisterSelection(PTSel);
  RegisterSelection(METSel);
  RegisterSelection(HTmuonSel);
  RegisterSelection(NJetSel);

  //RegisterSelection(HCALlaser);
  RegisterSelection(RazorSel);
  RegisterSelection(Chi2Seletion);
  RegisterSelection(Chi2NoTagSeletion);

  RegisterSelection(mttbar_gen_selection);
  RegisterSelection(BSel);
  RegisterSelection(NoBSel);
  // RegisterSelection(TopSel);
  RegisterSelection(TopTagSel);
  RegisterSelection(MuonSel);	     
  RegisterSelection(TwoDMuonSel);
  RegisterSelection(IsoConeSel);

  //TString unc_file = m_JECFileLocation + "/" + m_JECDataGlobalTag + "_Uncertainty_" + m_JECJetCollection + ".txt";
  //m_jes_unc = new JetCorrectionUncertainty(unc_file.Data());
  

 

  // ---------------- set up the histogram collections --------------------
  
  // histograms without any cuts

  RegisterHistCollection( new DelRHists("DelRHists", m_chi2discr));
  RegisterHistCollection( new DelRHists("AfterTagDelRHists", m_chi2discr));
  RegisterHistCollection( new MJetsHists("MJetsHists", m_chi2discr,10));

  RegisterHistCollection( new MJetsHists("Tag_muon_bjet", m_chi2discr,10));
  RegisterHistCollection( new MJetsHists("NoTag_muon_bjet", m_chi2discr,10));

  RegisterHistCollection( new MJetsHists("BSel", m_chi2discr,10));
  RegisterHistCollection( new MJetsHists("NoBSel", m_chi2discr,10));

  RegisterHistCollection( new MJetsHists("TagBSel", m_chi2discr,10));
  RegisterHistCollection( new MJetsHists("TagNoBSel", m_chi2discr,10));

  RegisterHistCollection( new MJetsHists("NoTagBSel", m_chi2discr,10));
  RegisterHistCollection( new MJetsHists("NoTagNoBSel", m_chi2discr,10));

  
  RegisterHistCollection( new MJetsHists("Control_one", m_tagchi2discr,10));
  RegisterHistCollection( new MJetsHists("Control_two", m_tagchi2discr,10));
  RegisterHistCollection( new MJetsHists("Control_third", m_tagchi2discr,10));
  

  RegisterHistCollection( new HypothesisHists("Chi2_BTag", m_chi2discr ) );

  RegisterHistCollection( new RazorHists("RazorHists", m_tagchi2discr));


  //Control Histograms
  //--------------------------------------------------------------

  //Control PTSel
  RegisterHistCollection( new EventHists(       "Event_PTSel") );
  RegisterHistCollection( new JetHists(          "Jets_PTSel") );
  RegisterHistCollection( new ElectronHists( "Electron_PTSel") );
  RegisterHistCollection( new MuonHists(         "Muon_PTSel") );
  RegisterHistCollection( new TauHists(           "Tau_PTSel") );
  RegisterHistCollection( new TopJetHists(    "TopJets_PTSel") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_PTSel") );

  //Control METSel
  RegisterHistCollection( new EventHists(       "Event_METSel") );
  RegisterHistCollection( new JetHists(          "Jets_METSel") );
  RegisterHistCollection( new ElectronHists( "Electron_METSel") );
  RegisterHistCollection( new MuonHists(         "Muon_METSel") );
  RegisterHistCollection( new TauHists(           "Tau_METSel") );
  RegisterHistCollection( new TopJetHists(    "TopJets_METSel") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_METSel") );

  //Control HTmuonSel
  RegisterHistCollection( new EventHists(       "Event_HTmuonSel") );
  RegisterHistCollection( new JetHists(          "Jets_HTmuonSel") );
  RegisterHistCollection( new ElectronHists( "Electron_HTmuonSel") );
  RegisterHistCollection( new MuonHists(         "Muon_HTmuonSel") );
  RegisterHistCollection( new TauHists(           "Tau_HTmuonSel") );
  RegisterHistCollection( new TopJetHists(    "TopJets_HTmuonSel") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_HTmuonSel") );


  //BTag
  RegisterHistCollection( new EventHists(       "Event_BTag") );
  RegisterHistCollection( new JetHists(          "Jets_BTag") );
  RegisterHistCollection( new ElectronHists( "Electron_BTag") );
  RegisterHistCollection( new MuonHists(         "Muon_BTag") );
  RegisterHistCollection( new TauHists(           "Tau_BTag") );
  RegisterHistCollection( new TopJetHists(    "TopJets_BTag") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_BTag") );

  //NoBTag
  RegisterHistCollection( new EventHists(       "Event_NoBTag") );
  RegisterHistCollection( new JetHists(          "Jets_NoBTag") );
  RegisterHistCollection( new ElectronHists( "Electron_NoBTag") );
  RegisterHistCollection( new MuonHists(         "Muon_NoBTag") );
  RegisterHistCollection( new TauHists(           "Tau_NoBTag") );
  RegisterHistCollection( new TopJetHists(    "TopJets_NoBTag") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_NoBTag") );
 

  //TopTag
  RegisterHistCollection( new EventHists(       "Event_TopTag") );
  RegisterHistCollection( new JetHists(          "Jets_TopTag") );
  RegisterHistCollection( new ElectronHists( "Electron_TopTag") );
  RegisterHistCollection( new MuonHists(         "Muon_TopTag") );
  RegisterHistCollection( new TauHists(           "Tau_TopTag") );
  RegisterHistCollection( new TopJetHists(    "TopJets_TopTag") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_TopTag") );

  //NoTopTag
  RegisterHistCollection( new EventHists(       "Event_NoTopTag") );
  RegisterHistCollection( new JetHists(          "Jets_NoTopTag") );
  RegisterHistCollection( new ElectronHists( "Electron_NoTopTag") );
  RegisterHistCollection( new MuonHists(         "Muon_NoTopTag") );
  RegisterHistCollection( new TauHists(           "Tau_NoTopTag") );
  RegisterHistCollection( new TopJetHists(    "TopJets_NoTopTag") );
  RegisterHistCollection( new JetEffiHists(   "JetEffi_NoTopTag") );

  
  //LumiControl
  RegisterHistCollection( new EventHists(       "Event_MJetsHists") );
  RegisterHistCollection( new EventHists(       "Event_TagBSel") );
  RegisterHistCollection( new EventHists(       "Event_TagNoBSel") );
  RegisterHistCollection( new EventHists(       "Event_NoTagBSel") );
  RegisterHistCollection( new EventHists(       "Event_NoTagNoBSel") );

  //Neutrino 
  RegisterHistCollection( new NeutrinoHists("Neutrino"        , m_chi2discr) );
  RegisterHistCollection( new NeutrinoHists("NeutrinoBSel"    , m_chi2discr) );
  RegisterHistCollection( new NeutrinoHists("NeutrinoNoBSel"  , m_chi2discr) );
  RegisterHistCollection( new NeutrinoHists("NeutrinoTopTag"  , m_chi2discr) );
  RegisterHistCollection( new NeutrinoHists("NeutrinoNoTopTag", m_chi2discr) );


  //ConstituentsHists

  RegisterHistCollection( new ConstituentsHists("Constituents_relIso05"   , m_tagchi2discr) );
  RegisterHistCollection( new ConstituentsHists("Constituents_before2D"   , m_tagchi2discr) );
  RegisterHistCollection( new ConstituentsHists("Constituents_after2D"    , m_tagchi2discr) );
  RegisterHistCollection( new ConstituentsHists("Constituents_afterCuts"  ,m_chi2discr) );

  RegisterHistCollection( new MetaTreeFiller("MetaTree",m_tagchi2discr, m_tree) );
  RegisterHistCollection( new CleanerFiller("Cleaner",m_chi2discr, m_tree) );

  //RegisterHistCollection( new TopTagDelRHists("TopTagDelRHists", m_chi2discr));

  InitHistos();
  
  m_bsf = NULL;
  std::transform(m_dobsf.begin(), m_dobsf.end(), m_dobsf.begin(), ::tolower);
  if(m_dobsf != "none"&& m_addGenInfo) {
    E_SystShift sys_bjets = e_Default;
    E_SystShift sys_ljets = e_Default;
    if (m_dobsf == "default") {
      m_logger << INFO << "Applying btagging scale factor" << SLogger::endmsg;
    } else if (m_dobsf == "up-bjets") {
      m_logger << INFO << "Applying btagging up scale factor for b-jets" << SLogger::endmsg;
      sys_bjets = e_Up;
    } else if (m_dobsf == "down-bjets") {
      m_logger << INFO << "Applying btagging down scale factor for b-jets" << SLogger::endmsg;
      sys_bjets = e_Down;
    } else if (m_dobsf == "up-ljets") {
      m_logger << INFO << "Applying btagging up scale factor for l-jets" << SLogger::endmsg;
      sys_ljets = e_Up;
    } else if (m_dobsf == "down-ljets") {
      m_logger << INFO << "Applying btagging down scale factor for l-jets" << SLogger::endmsg;
      sys_ljets = e_Down;
    }
    else
      m_logger << ERROR << "Unknown BTaggingScaleFactors option, default option is applied --- should be either `Default`, `Up-bjets`, `Down-bjets`, `Up-ljets`, or `Down-ljets`" << SLogger::endmsg;
    if(doEle)
      m_bsf = new BTaggingScaleFactors(m_btagtype, e_Electron, sys_bjets, sys_ljets);
    else if(doMu)
      m_bsf = new BTaggingScaleFactors(m_btagtype, e_Muon, sys_bjets, sys_ljets);
  }
  
  
  return;

}

void QCDCycle::EndInputData( const SInputData& id ) throw( SError ) 
{
  AnalysisCycle::EndInputData( id );


  return;

}

void QCDCycle::BeginInputFile( const SInputData& id ) throw( SError ) 
{
  // Connect all variables from the Ntuple file with the ones needed for the analysis
  // The variables are commonly stored in the BaseCycleContaincer

  // important: call to base function to connect all variables to Ntuples from the input tree
  AnalysisCycle::BeginInputFile( id );

  return;

}

void QCDCycle::ExecuteEvent( const SInputData& id, Double_t weight) throw( SError ) 
{
  // this is the most important part: here the full analysis happens
  // user should implement selections, filling of histograms and results

  // first step: call Execute event of base class to perform basic consistency checks
  // also, the good-run selection is performed there and the calculator is reset
  AnalysisCycle::ExecuteEvent( id, weight );

  // get the selections
  

  //static Selection* TwoDMuonSel = GetSelection("TwoDSelection");
  static Selection* BSel = GetSelection("BSelection");
  static Selection* MuonSel = GetSelection("MuonSelection");
  static Selection* NoBSel = GetSelection("NoBSelection");
  //static Selection* TopSel = GetSelection("TopSelection");
  //static Selection* TTreco = GetSelection("TTreco");
  static Selection* TopTagSel = GetSelection("TopTagSelection");
  static Selection* TwoDMuon = GetSelection("TwoDSelection");
  static Selection* HCALlaser = GetSelection("HCAL_laser_events");
  static Selection* ChiSelection = GetSelection("Chi2Selection");
  static Selection* ChiNoTag = GetSelection("Chi2NoTagSelection");

  static Selection* IsoSel = GetSelection("IsoConeSelection");

  static Selection* PTSel = GetSelection("PTSelection");
  static Selection* METSel = GetSelection("METSelelection");
  static Selection* HTmuonSel = GetSelection("HTmuonSelelection");

  static Selection* NJetSel = GetSelection("NJetSelection");

  EventCalc* calc = EventCalc::Instance();
  BaseCycleContainer* bcc = calc->GetBaseCycleContainer();


  

  TopFitCalc* topfit = TopFitCalc::Instance();

  //if(!TwoDMuonSel->passSelection())  throw SError( SError::SkipEvent );
 

  




  if(!MuonSel->passSelection()) ClearEvent();
  //static Selection* mttbar_gen_selection = GetSelection("Mttbar_Gen_Selection");
  //if(mttbar_gen_selection->passSelection()) ClearEvent();

  //if(calc->GetMuons()->at(0).relIso() > 0.4) throw SError( SError::SkipEvent );

  /*	
  // reject laser events only for data
  if (calc->IsRealData()){
    if (!HCALlaser->passSelection()) throw SError( SError::SkipEvent );
  }
  */

  
  m_cleaner = new Cleaner();
  m_cleaner->SetJECUncertainty(m_jes_unc);


  // settings for jet correction uncertainties
  if (m_sys_unc==e_JEC){
    if (m_sys_var==e_Up) m_cleaner->ApplyJECVariationUp();
    if (m_sys_var==e_Down) m_cleaner->ApplyJECVariationDown();
  }
  if (m_sys_unc==e_JER){
    if (m_sys_var==e_Up) m_cleaner->ApplyJERVariationUp();
    if (m_sys_var==e_Down) m_cleaner->ApplyJERVariationDown();
  }


  //if(bcc->jets) m_cleaner->JetCleaner(50,2.5,true);
  //do reconstruction here
  /*
  if(bcc->jets) m_cleaner->JetCleaner(50,2.5,true);
  
  if(bcc->taus) m_cleaner->TauCleaner(double_infinity(),0.0);
  if(!TopSel->passSelection()) ClearEvent();
  */
  
  
  bcc->recoHyps->clear();    
 
  //bool topfit_flag = false;
   

  if(TopTagSel->passSelection()){
    topfit->CalculateTopTag();
  } else{
    topfit->FillHighMassTTbarHypotheses();
  }

  if(!bcc->recoHyps || bcc->recoHyps->size()==0) ClearEvent();
 


  m_tagchi2discr->FillDiscriminatorValues();
   
  
  static Selection* RazorSelection = GetSelection("RazorSelection");
  BaseHists* Control_one   = GetHistCollection("Control_one");
  BaseHists* Control_two   = GetHistCollection("Control_two");
  BaseHists* Control_third = GetHistCollection("Control_third");
 
  BaseHists* Constituents_before2D = GetHistCollection("Constituents_before2D");
  BaseHists* Constituents_after2D = GetHistCollection("Constituents_after2D");
  /*
  BaseHists* Constituents_relIso05 = GetHistCollection("Constituents_relIso05");
  BaseHists* MetaTree = GetHistCollection("MetaTree");

  ReconstructionHypothesis *discr = m_tagchi2discr->GetBestHypothesis();
  
  BaseHists* RazorHists = GetHistCollection("RazorHists");
  */
  
  
  Muon muon = bcc->muons->at(0);
  
  /*
  std::vector< Jet >* antikjets_before = bcc->jets;
  Particle blep_discr;
  blep_discr.set_v4(discr->blep_v4());
  
  LorentzVector toplep = discr->toplep_v4();
  LorentzVector tophad = discr->tophad_v4();

  double exp_tophad_mass = 181;
  double exp_tophad_sig = 15;
  double exp_toplep_mass = 174;
  double exp_toplep_sig = 18;

  double test = sqrt(pow(0.5*(toplep.M2()+tophad.M2() - exp_tophad_mass*exp_tophad_mass - exp_toplep_mass*exp_toplep_mass),2))/1000;
  

  if(TopTagSel->passSelection()){
    //if(test < 40 && pTrel(muon,blep_discr)< 15) ClearEvent();
    if((pTrel(muon,blep_discr)<20 && deltaRmin(&muon,antikjets_before)<0.2) || deltaRmin(&muon,antikjets_before)<0.1) ClearEvent();
  }
  else{
    if(!TwoDMuon->passSelection()) ClearEvent();
  }
  */
  
  
  //if((toplep+tophad).M()<600) ClearEvent();
  //if(test < 40 && pTrel(muon,blep_discr)< 15) ClearEvent();
  //if((pTrel(muon,blep_discr)<20 && deltaRmin(&muon,antikjets_before)<0.2) || deltaRmin(&muon,antikjets_before)<0.1) ClearEvent();

  //Constituents_before2D->Fill();
  //MetaTree->Fill();


  /*
  if(muon.pt()<150){
    if(relIsoMuon(muon,.4)>0.5) ClearEvent();
  }
  else if(muon.pt()>150){
    if(relIsoMuon(muon,0.1)>0.1) ClearEvent();
  }
  */
  
  double a = 29.1356;
  double b = 164.383;
  double c = 0.023111;
  double y = a/(muon.pt()+b)+c;
 
  if(y<0.02) y=0.02;

  if(!IsoSel->passSelection()) ClearEvent();




  //if(relIsoMuon(muon,0.5)>0.2) Constituents_relIso05->Fill();

  //if(relIsoMuon(muon,y)>0.2 && relIsoMuon(muon,0.4)>0.2) ClearEvent();
  

  //if(relIsoMuon(muon,0.5)>0.2 && relIsoMuon(muon,0.4)>0.2 && relIsoMuon(muon,0.08)>0.2) ClearEvent();



  //if(!TwoDMuon->passSelection()) ClearEvent();

  //Constituents_after2D->Fill();

  //if(!RazorSelection->passSelection()) ClearEvent();

  
  
  if(bcc->jets) m_cleaner->JetCleaner(50,2.5,true);
 




  if(muon.pt()>250 && muon.pt() <350 )Constituents_after2D->Fill();


  
  //RazorHists->Fill();
  //FillControlHistos("_PTSel");
  //Control_one->Fill();
  if(!PTSel->passSelection()) ClearEvent();
  //FillControlHistos("_METSel");
  //Control_two->Fill();
  if(!METSel->passSelection()) ClearEvent();
  //FillControlHistos("_HTmuonSel");
  //Control_third->Fill();
  if(!HTmuonSel->passSelection()) ClearEvent();
  

  

  if(bcc->taus) m_cleaner->TauCleaner(double_infinity(),0.0);
  
  // b tagging scale factor
  //if(m_bsf && m_addGenInfo) {
  //  calc->ProduceWeight(m_bsf->GetWeight());
  //}
  if(!NJetSel->passSelection()) ClearEvent();
  

  bcc->recoHyps->clear();    
 
  


  //bool topfit_flag = false;
  
  if(TopTagSel->passSelection()){
   
    //topfit->FillHighMassTTbarHypotheses();
    topfit->CalculateTopTag();
    
  }
  else{
    //ClearEvent();
    //topfit->CalculateSelection();
    topfit->FillHighMassTTbarHypotheses();
  }

   
  if(!bcc->recoHyps || bcc->recoHyps->size()==0) ClearEvent();
   
  //if((pTrel(muon,blep_discr)<20 && deltaRmin(&muon,antikjets_before)<0.2) || deltaRmin(&muon,antikjets_before)<0.1) ClearEvent();

  m_chi2discr->FillDiscriminatorValues();

  


  //WriteOutputTree();


  //return;




  //BaseHists* Cleaner = GetHistCollection("Cleaner");
  //Cleaner->Fill();


  BaseHists* Constituents_afterCuts = GetHistCollection("Constituents_afterCuts");


  Constituents_afterCuts->Fill();

  BaseHists* DeltaRHists = GetHistCollection("DelRHists");
  BaseHists* AfterTagDeltaRHists = GetHistCollection("AfterTagDelRHists");

  BaseHists* MJetsHists = GetHistCollection("MJetsHists");

  BaseHists* Tag_Muon_bjet = GetHistCollection("Tag_muon_bjet");
  BaseHists* NoTag_Muon_bjet = GetHistCollection("NoTag_muon_bjet");

  BaseHists* BSelH = GetHistCollection("BSel");
  BaseHists* NoBSelH = GetHistCollection("NoBSel");

  BaseHists* TagBSelH = GetHistCollection("TagBSel");
  BaseHists* TagNoBSelH = GetHistCollection("TagNoBSel");

  BaseHists* NoTagBSelH = GetHistCollection("NoTagBSel");
  BaseHists* NoTagNoBSelH = GetHistCollection("NoTagNoBSel");


  BaseHists* Chi2_HistsBTag = GetHistCollection("Chi2_BTag");

  BaseHists* Neutrinos = GetHistCollection("Neutrino");
  BaseHists* NeutrinosBSel = GetHistCollection("NeutrinoBSel");
  BaseHists* NeutrinosNoBSel = GetHistCollection("NeutrinoNoBSel");
  BaseHists* NeutrinosTopTag = GetHistCollection("NeutrinoTopTag");
  BaseHists* NeutrinosNoTopTag = GetHistCollection("NeutrinoNoTopTag");

  //BaseHists* TopTagDeltaRHists = GetHistCollection("TopTagDelRHists");
  
  BaseHists* Event_MJetsHists = GetHistCollection("Event_MJetsHists");
  BaseHists* Event_TagBSel = GetHistCollection("Event_TagBSel");
  BaseHists* Event_TagNoBSel = GetHistCollection("Event_TagNoBSel");
  BaseHists* Event_NoTagBSel = GetHistCollection("Event_NoTagBSel");
  BaseHists* Event_NoTagNoBSel = GetHistCollection("Event_NoTagNoBSel");

   //DeltaRHists->Fill();
  
  std::vector<TopJet>* cajets = calc->GetCAJets();
  int NCAJets = cajets->size();


  if(NCAJets==0)MJetsHists->Fill();

  
  //Neutrinos->Fill();
  //Event_MJetsHists->Fill();

  //if(!ChiSelection->passSelection()) ClearEvent();

  if(BSel->passSelection() && ChiSelection->passSelection()) BSelH->Fill();
  if(BSel->passSelection() && ChiSelection->passSelection()) FillControlHistos("_BTag");
  if(BSel->passSelection() && ChiSelection->passSelection()) Chi2_HistsBTag->Fill();
  if(BSel->passSelection() && ChiSelection->passSelection()) NeutrinosBSel->Fill();
  if(NoBSel->passSelection() && ChiNoTag->passSelection() ) NoBSelH->Fill();
  if(NoBSel->passSelection() && ChiNoTag->passSelection() ) FillControlHistos("_NoBTag");
  if(NoBSel->passSelection() && ChiNoTag->passSelection() ) NeutrinosNoBSel->Fill();
  //if(topfit_flag)TopTagDeltaRHists->Fill();


  if(!TopTagSel->passSelection() && ChiNoTag->passSelection()){
    NoTag_Muon_bjet->Fill();
    FillControlHistos("_NoTopTag");
    if(BSel->passSelection()) {
      NoTagBSelH->Fill();
      Event_NoTagBSel->Fill();
    }
    if(NoBSel->passSelection()){ 
      NoTagNoBSelH->Fill();
      Event_NoTagNoBSel->Fill();
    }
    NeutrinosNoTopTag->Fill();

  }
  
  if(!TopTagSel->passSelection() || !ChiSelection->passSelection()) ClearEvent();

  FillControlHistos("_TopTag");
  NeutrinosTopTag->Fill();

  //AfterTagDeltaRHists->Fill();
  Tag_Muon_bjet->Fill();
  
  if(BSel->passSelection()) {
    TagBSelH->Fill();
    Event_TagBSel->Fill();
  }
  if(NoBSel->passSelection()){
    TagNoBSelH->Fill();
    Event_TagNoBSel->Fill();
  }
  ClearEvent();

  
}



void QCDCycle::FillControlHistos(TString postfix)
{
    // fill some control histograms, need to be defined in BeginInputData

    BaseHists* eventhists = GetHistCollection((std::string)("Event"+postfix));
    BaseHists* jethists = GetHistCollection((std::string)("Jets"+postfix));
    BaseHists* elehists = GetHistCollection((std::string)("Electron"+postfix));
    BaseHists* muonhists = GetHistCollection((std::string)("Muon"+postfix));
    BaseHists* tauhists = GetHistCollection((std::string)("Tau"+postfix));
    BaseHists* topjethists = GetHistCollection((std::string)("TopJets"+postfix));
    BaseHists* jeteffihists = GetHistCollection((std::string)("JetEffi"+postfix));

    jeteffihists->Fill();
    eventhists->Fill();
    jethists->Fill();
    elehists->Fill();
    muonhists->Fill();
    tauhists->Fill();
    topjethists->Fill();
}
