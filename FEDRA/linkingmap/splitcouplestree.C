//split couplestrees. Opposite of merge_trees. Instead of merging many couples into one file, it splits one file in multiple couples
//created 5 June 2023 by A.Iuliano
void splitcouplestree(int brickID, int plateID, int testcellix, int testcelliy){
 //histo file
 MemInfo_t *memory_info = new MemInfo_t();
 //histograms to be stored
 TStopwatch *timing = new TStopwatch();
 timing->Start();

 //same parameters used as input for linking map!
 const int nx = 18;
 const int ny = 18;
 const float xmin = 5000.;
 const float xmax = 185000.;
 const float ymin = 5000.;
 const float ymax = 185000.;
 const float overlapfraction = 0.25;
 //test only one cell
 //const int testcellix = 10;
 //const int testcelliy = 10;
 //const float overlapfraction = 0.; //try without overlap
 EdbCell2 *emulsioncell = new EdbCell2();
 emulsioncell->InitCell(nx,xmin,xmax,ny,ymin,ymax,1);
 
 //bin width
 float xbin = emulsioncell->Xbin();
 float ybin = emulsioncell->Ybin();
 cout<<"creating output files"<<endl;
 EdbCouplesTree *outputtree[nx][ny];
 for (int ix = 0; ix < nx; ix++){
  for (int iy = 0; iy < ny; iy++){
   //couples to write into
   if(ix!= testcellix || iy != testcelliy) continue;
   outputtree[ix][iy] = new EdbCouplesTree();
   outputtree[ix][iy]->InitCouplesTree("couples",Form("/eos/experiment/sndlhc/emulsionData/2022/emureco_Napoli/RUN1/b%0*i/p%0*i/%i.%i.%i.%i.cp.root",6,brickID,3,plateID,brickID,plateID,ix+1,iy+1),"RECREATE");
  }
 }
 cout<<"reading input tree"<<endl;
 EdbCouplesTree *mytree = new EdbCouplesTree();
 mytree->InitCouplesTree("couples",Form("/eos/experiment/sndlhc/emulsionData/2022/emureco_Napoli/RUN1/b%0*i/p%0*i/%i.%i.0.0.cp.root",6,brickID,3,plateID,brickID,plateID),"READ");
 mytree->eCut = "eCHI2P<2.4&&s.eW>20&&eN1<=1&&eN2<=1&&s1.eFlag>=0&&s2.eFlag>=0"; //selecting couples for tracking

 TEventList *lst = mytree->InitCutList();
 int nlst =lst->GetN();
 cout<<"We have "<<nlst<<" good couples"<<endl;

 int entr = 0;
 for(int i=0; i<nlst; i++ ) {
  entr = lst->GetEntry(i);
  mytree->GetEntry(entr);

  float xseg = mytree->eS->X();
  float yseg = mytree->eS->Y();

  //finding cell the couple belongs to
  int ix = emulsioncell->IX(xseg);
  int iy = emulsioncell->IY(yseg);

  if (ix < testcellix - 1 || ix > testcellix + 1 || iy < testcelliy - 1 || iy > testcelliy + 1) continue; //do not do anything

  if(ix== testcellix && iy == testcelliy){

  EdbSegP *seg_bt = new EdbSegP(); //these must be deleted or not in the loop? We need to write them, so I would say NO
  EdbSegP *seg_s1 = new EdbSegP();
  EdbSegP *seg_s2 = new EdbSegP();

  seg_bt->Copy(*mytree->eS);
  seg_s1->Copy(*mytree->eS1);
  seg_s2->Copy(*mytree->eS2);

  outputtree[ix][iy]->Fill(seg_s1, seg_s2, seg_bt); 
  } //end if cell
  //overlapping cells? check if close to border
  if(ix>0){ //cell to the left exists
   if (xseg<(emulsioncell->X(ix) - xbin/2.*(1 - overlapfraction))){ //close to the left border enough
    if(ix-1== testcellix && iy == testcelliy){
      EdbSegP *overlap_bt = new EdbSegP(); //these must be deleted or not in the loop? We need to write them, so I would say NO
      EdbSegP *overlap_s1 = new EdbSegP();
      EdbSegP *overlap_s2 = new EdbSegP();

      overlap_bt->Copy(*mytree->eS);
      overlap_s1->Copy(*mytree->eS1);
      overlap_s2->Copy(*mytree->eS2);

      outputtree[ix-1][iy]->Fill(overlap_s1, overlap_s2, overlap_bt); //ordine s1 s2 s!
    } //end if cell
   } //end overlap check
  } //end cell check

  if(ix<(nx-1)){ //cell to the right exists
   if (xseg>(emulsioncell->X(ix) + xbin/2.*(1 - overlapfraction))){ //close to the left border enough
    if(ix+1 == testcellix && iy == testcelliy){
      EdbSegP *overlap_bt = new EdbSegP(); //these must be deleted or not in the loop? We need to write them, so I would say NO
      EdbSegP *overlap_s1 = new EdbSegP();
      EdbSegP *overlap_s2 = new EdbSegP();

      overlap_bt->Copy(*mytree->eS);
      overlap_s1->Copy(*mytree->eS1);
      overlap_s2->Copy(*mytree->eS2);

      outputtree[ix+1][iy]->Fill(overlap_s1, overlap_s2, overlap_bt); //ordine s1 s2 s!
    } //end if
   } //end overlap check
  }//end cell check

  if(iy>0){ //cell above exists
   if (yseg<(emulsioncell->Y(iy) - ybin/2.*(1 - overlapfraction))){ //close below exists
    if(ix== testcellix && iy-1 == testcelliy){ 
      EdbSegP *overlap_bt = new EdbSegP(); //these must be deleted or not in the loop? We need to write them, so I would say NO
      EdbSegP *overlap_s1 = new EdbSegP();
      EdbSegP *overlap_s2 = new EdbSegP();

      overlap_bt->Copy(*mytree->eS);
      overlap_s1->Copy(*mytree->eS1);
      overlap_s2->Copy(*mytree->eS2);

      outputtree[ix][iy-1]->Fill(overlap_s1, overlap_s2, overlap_bt); //ordine s1 s2 s!
    } //end if
   } //end overlap check
  } //end cell check

  if(iy<(ny-1)){ //cell below exists
   if (yseg>(emulsioncell->Y(iy) + ybin/2.*(1 - overlapfraction))){ //close above exists
    if(ix== testcellix && iy + 1 == testcelliy){ 
      EdbSegP *overlap_bt = new EdbSegP(); //these must be deleted or not in the loop? We need to write them, so I would say NO
      EdbSegP *overlap_s1 = new EdbSegP();
      EdbSegP *overlap_s2 = new EdbSegP();

      overlap_bt->Copy(*mytree->eS);
      overlap_s1->Copy(*mytree->eS1);
      overlap_s2->Copy(*mytree->eS2);

      outputtree[ix][iy+1]->Fill(overlap_s1, overlap_s2, overlap_bt); //ordine s1 s2 s!
    } //end if
   } //end overlap check
  }//end cell check

 } //end loop
 timing->Stop();
 timing->Print();

 gSystem->GetMemInfo(memory_info);
 cout<<"Used memory "<<memory_info->fMemUsed<<endl;

 for (int ix = 0; ix < nx; ix++){
  for (int iy = 0; iy < ny; iy++){
   if(ix!= testcellix || iy != testcelliy) continue;
   outputtree[ix][iy]->Close();
  }
 }

}
