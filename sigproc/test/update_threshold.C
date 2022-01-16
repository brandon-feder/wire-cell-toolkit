void update(){
  TFile *file = new TFile("l1.root","update");
  TH1I *hu_threshold = new TH1I("hu_threshold","hu_threshold",2400,-0.5,-0.5+2400);
  TH1I *hv_threshold = new TH1I("hv_threshold","hv_threshold",2400,2400-0.5,-0.5+4800);
  TH1I *hw_threshold = new TH1I("hw_threshold","hw_threshold",3456,4800-0.5,4800-0.5+3456);
  for (int i=0;i!=2400;i++){
    hu_threshold->SetBinContent(i+1,1600/4);
    hv_threshold->SetBinContent(i+1,2000/4);
  }
  for (int i=0;i!=3456;i++){
    hw_threshold->SetBinContent(i+1,800/4);
  }
  hu_threshold->SetDirectory(file);
  hv_threshold->SetDirectory(file);
  hw_threshold->SetDirectory(file);
  
  file->Write();
  file->Close();
}
