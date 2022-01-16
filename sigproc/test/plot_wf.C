void plot_wf(Int_t channel = 100){
  TFile *file1 = new TFile("nsp_2D_display_3455_0_6.root");
  TFile *file2 = new TFile("temp.root");
  TFile *file3 = new TFile("../toolkit_old/temp.root");

  TH2F *h1, *h2, *h3;
  if (channel <2400){
    h1 = (TH2F*)file1->Get("hu_raw");
    h2 = (TH2F*)file2->Get("hu_raw");
    h3 = (TH2F*)file3->Get("hu_raw");
  }else if (channel < 4800){
    h1 = (TH2F*)file1->Get("hv_raw");
    h2 = (TH2F*)file2->Get("hv_raw");
    h3 = (TH2F*)file3->Get("hv_raw");
    channel -=2400;
  }else{
    h1 = (TH2F*)file1->Get("hw_raw");
    h2 = (TH2F*)file2->Get("hw_raw");
    h3 = (TH2F*)file3->Get("hw_raw");
    channel -= 4800;
  }
  Int_t nticks = h1->GetNbinsY();

  TH1F *h10 = new TH1F("h10","h10",nticks,0,nticks);
  TH1F *h20 = new TH1F("h20","h20",nticks,0,nticks);
  TH1F *h30 = new TH1F("h30","h30",nticks,0,nticks);
  for (Int_t i=0;i!=nticks;i++){
    h10->SetBinContent(i+1,h1->GetBinContent(channel+1,i+1));
    h20->SetBinContent(i+1,h2->GetBinContent(channel+1,i+1));
    h30->SetBinContent(i+1,h3->GetBinContent(channel+1,i+1));
  }

  TCanvas *c1 = new TCanvas("c1","c1",1200,600);
  c1->Divide(2,1);
  
  c1->cd(1);
  h10->Draw();
  h20->Draw("same");
  h30->Draw("same");
  h10->SetLineColor(1);
  h20->SetLineColor(2);
  h30->SetLineColor(4);
  
  h20->SetLineStyle(2);
  h30->SetLineStyle(3);
  TLegend *le1 = new TLegend(0.6,0.6,0.89,0.89);
  le1->SetFillColor(10);
  le1->AddEntry(h10,"WCP","l");
  le1->AddEntry(h20,"new WCT","l");
  le1->AddEntry(h30,"old WCT","l");
  le1->Draw();

  c1->cd(2);
  TH1F *h40 = (TH1F*)h20->Clone("h40");
  h40->Add(h30,-1);
  h40->Draw();
}
