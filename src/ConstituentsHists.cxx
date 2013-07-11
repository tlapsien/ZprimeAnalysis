#include "include/ConstituentsHists.h"
#include "include/EventCalc.h"
#include "include/TopFitCalc.h"
#include <iostream>
#include <cmath>
#include <stdio.h>

using namespace std;

ConstituentsHists::ConstituentsHists(const char* name,  HypothesisDiscriminator *discr) : BaseHists(name)
{
  // named default constructor
  m_discr = discr;
}

ConstituentsHists::~ConstituentsHists()
{
  // default destructor, does nothing
}

void ConstituentsHists::Init()
{
  // book all histograms here

  // leptons
  Book( TH1F("mu_size"  ,"Number of #mu" ,5,-.5,4.5));
  Book( TH1F("mu_pt_ly"  ," #mu p_{T}" ,50,0,900));
  Book( TH1F("mu_eta"  ," #eta #mu" ,50,-4,4));
  Book( TH1F("mu_phi"  ," #phi #mu" ,50,-PI,PI));
  Book( TH1F("mu_reliso"  ," relIso #mu" ,50,0,5));

  // jets
  Book( TH1F("mu_nextJet_pt_ly"  ," p_{T} nextJet to #mu" ,50,0,600));
  Book( TH1F("mu_nextJet_phi"  ,"#phi nextJet to #mu" ,50,-PI,PI));
  Book( TH1F("mu_nextJet_eta"  ,"#eta nextJet to #mu" ,50,-4,4));
  Book( TH1F("mu_nextJet_delR"  ,"#Delta R nextJet to #mu " ,50,0,6));

  Book( TH1F("mu_nextJet_area_num"  ,"Area/Number of Constituent fom nextJet to #mu " ,100,0,.2));
  Book( TH1F("neutrino_nextJet_area_num"  ,"Area/Number of Constituent fom nextJet to #nu " ,100,0,.2));

  // constituents
  Book( TH1F("con_size"  ,"Number of Constituents" ,120,-.5,119.5));
  Book( TH1F("con_pt_ly"  ,"Constituents  p_{T}" ,50,0,100));
  Book( TH1F("con_eta"  ," #eta Constituents" ,50,-4,4));
  Book( TH1F("con_phi"  ," #phi Constituents" ,50,-PI,PI));

  Book( TH1F("con_muondelphi"  ,"#Delta #phi muon-Constituents" ,100,0,PI/2));
  Book( TH1F("con_muondelR"  ," #Delta R muon-Constituents" ,100,0,2.5));

  Book( TH1F("con_neutrinodelphi"  ,"#Delta #phi #nu -Constituents" ,100,0,PI));
  Book( TH1F("con_neutrinodelR_ly"  ," #Delta R #nu -Constituents" ,100,0,5));
  Book( TH1F("filter"  ," ",100,40,900));


  Book( TH2F("pf_con_pt_mu_delR","",100,0,150,100,0,5));
  Book( TH2F("pf_con_pt_nu_delR","",100,0,150,100,0,5));
	
	
}

void ConstituentsHists::Fill()
{
  // fill the histograms


  EventCalc* calc = EventCalc::Instance();
  bool IsRealData = calc->IsRealData();
  TopFitCalc* fitcalc = TopFitCalc::Instance();


  BaseCycleContainer* bcc = calc->GetBaseCycleContainer();

  LuminosityHandler* lumih = calc->GetLumiHandler();

  ReconstructionHypothesis* hyp = m_discr->GetBestHypothesis();

  // important: get the event weight
  double weight = calc->GetWeight();

  std::vector<Muon>* muons = calc->GetMuons();
  
  std::vector<TopJet>* cajets = calc->GetCAJets();
  std::vector<Jet>* antikjets = calc->GetJets();

  Particle neutrino; 
  neutrino.set_v4(hyp->neutrino_v4());

  Particle wlep;
  wlep.set_v4(hyp->wlep_v4());


  double chi_toplep = 9999.;
  unsigned int bjet_num=-1;

  /*
  for(unsigned int i = 0; i < antikjets->size(); ++i){
    if(chi_toplep > pow((antikjets->at(0).v4()+ wlep.v4()).M() - 174,2.) ){
      chi_toplep = pow(antikjets->at(0).v4()+ wlep.v4() - 174,2.);
      bjet_num= i;
    } 
  }
  */  


  int Nmuons = muons->size();
  Hist("mu_size")->Fill(Nmuons, weight);
 

  for (int i=0; i<Nmuons; ++i){
    Muon muon = muons->at(i);

    Jet* Mu_netxtJet = nextJet(&muon,antikjets);
    Jet* Nu_netxtJet = nextJet(&neutrino,antikjets);

    std::vector<PFParticle>* pfcon_list = calc->GetJetPFParticles(Mu_netxtJet,0.8);
    
    Hist("con_size")->Fill(pfcon_list->size(), weight);
    Hist("mu_pt_ly")->Fill(muon.pt(), weight);
    Hist("mu_eta")->Fill(muon.eta(), weight);
    Hist("mu_phi")->Fill(muon.phi(), weight);
    Hist("mu_reliso")->Fill(muon.relIso(), weight);

    Hist("mu_nextJet_pt_ly")->Fill(nextJet(&muon,antikjets)->pt(),weight);
    Hist("mu_nextJet_phi")->Fill(nextJet(&muon,antikjets)->phi(),weight);
    Hist("mu_nextJet_eta")->Fill(nextJet(&muon,antikjets)->eta(),weight);
    Hist("mu_nextJet_delR")->Fill(deltaRmin(&muon,antikjets),weight);


    Hist("mu_nextJet_area_num")->Fill(Mu_netxtJet->jetArea()/pfcon_list->size(),weight);
    Hist("neutrino_nextJet_area_num")->Fill(Nu_netxtJet->jetArea()/pfcon_list->size(),weight);

    double count =0;	

    for(unsigned int u = 0; u<pfcon_list->size();++u){
      Particle pfcon = pfcon_list->at(u);
      
      if(pfcon.deltaR(muon)<.5) count+=1;


      Hist("con_muondelR")->Fill(pfcon.deltaR(muon));
      Hist("con_neutrinodelR_ly")->Fill(pfcon.deltaR(neutrino));
      Hist("con_muondelphi")->Fill(pfcon.deltaPhi(muon));
      Hist("con_neutrinodelphi")->Fill(pfcon.deltaPhi(neutrino));
       
      Hist("pf_con_pt_mu_delR")->Fill(pfcon.pt(),pfcon.deltaR(muon));
      Hist("pf_con_pt_nu_delR")->Fill(pfcon.pt(),pfcon.deltaR(neutrino));


      Hist("con_pt_ly")->Fill(pfcon.pt(), weight);
      Hist("con_eta")->Fill(pfcon.eta(), weight);
      Hist("con_phi")->Fill(pfcon.phi(), weight);
    }

    if(count/((double) pfcon_list->size())>.8)
      Hist("filter")->Fill(muon.pt(),weight);

  }


}

void ConstituentsHists::Finish()
{
  // final calculations, like division and addition of certain histograms

}

