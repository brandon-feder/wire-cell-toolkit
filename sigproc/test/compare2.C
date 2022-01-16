void compare2(){
  TFile *file1 = new TFile("../toolkit_old/temp.root");
  //TFile *file1 = new TFile("nsp_2D_display_3455_0_6.root");
  //TFile *file1 = new TFile("2D_display_3493_821_0.root");
  //TFile *file1 = new TFile("2D_display_3469_1064_0.root");
  TFile *file2 = new TFile("temp.root");
  TH2F *hu1 = (TH2F*)file1->Get("hu_raw");
  TH2F *hu2 = (TH2F*)file2->Get("hu_raw");
  TH1F *hu = new TH1F("hu","hu",2400, 0,2400);

  TH2F *hv1 = (TH2F*)file1->Get("hv_raw");
  TH2F *hv2 = (TH2F*)file2->Get("hv_raw");
  TH1F *hv = new TH1F("hv","hv",2400, 0,2400);
  
  TH2F *hw1 = (TH2F*)file1->Get("hw_raw");
  TH2F *hw2 = (TH2F*)file2->Get("hw_raw");
  TH1F *hw = new TH1F("hw","hw",3456, 0,3456);
  
  TCanvas *c1 = new TCanvas("c1","c1",800,800);
  c1->Divide(2,2);
  c1->cd(1);
  
  for (Int_t i=0;i!=2400;i++){
    Double_t sum = 0;
    for (Int_t j=0;j!=9594;j++){
      sum += fabs(hu1->GetBinContent(i+1,j+1) - hu2->GetBinContent(i+1,j+1));
    }
    hu->SetBinContent(i+1,sum);
  }
  hu->Draw();
  
  c1->cd(2);
  for (Int_t i=0;i!=2400;i++){
    Double_t sum = 0;
    for (Int_t j=0;j!=9594;j++){
      sum += fabs(hv1->GetBinContent(i+1,j+1) - hv2->GetBinContent(i+1,j+1));
    }
    hv->SetBinContent(i+1,sum);
  }
  hv->Draw();

  c1->cd(3);
  for (Int_t i=0;i!=3456;i++){
    Double_t sum = 0;
    for (Int_t j=0;j!=9594;j++){
      sum += fabs(hw1->GetBinContent(i+1,j+1) - hw2->GetBinContent(i+1,j+1));
    }
    hw->SetBinContent(i+1,sum);
  }
  hw->Draw();
}
