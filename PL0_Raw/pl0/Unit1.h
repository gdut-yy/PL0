//---------------------------------------------------------------------------
#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <stdio.h>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TButton *ButtonRun;
    TMemo *Memo1;
    TEdit *EditName;
    TLabel *Label1;
    TRadioGroup *ListSwitch;
    void __fastcall ButtonRunClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TForm1(TComponent* Owner);
    void __fastcall printfs(char* s);
    void __fastcall printcs(char c);
    void __fastcall printls(char *s0, long v);
    void __fastcall printrs(char *s0, float v);
    void __fastcall prinths(char* s0, short v);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::printfs(char* s) {
    Memo1->Lines->Add(s);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::printcs(char c) {
    Memo1->Lines->Add(c);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::printls(char *s0, long v) {
    Memo1->Lines->Add(s0 + IntToStr(v));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::printrs(char *s0, float v) {
    Memo1->Lines->Add(s0 + FloatToStr(v));
}
//---------------------------------------------------------------------------
#endif
