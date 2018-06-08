object Form1: TForm1
  Left = -4
  Top = -4
  Width = 1032
  Height = 746
  Align = alLeft
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 832
    Top = 32
    Width = 160
    Height = 40
    Caption = #28304#31243#24207#21517
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -40
    Font.Name = #40657#20307
    Font.Style = []
    ParentFont = False
  end
  object ButtonRun: TButton
    Left = 844
    Top = 395
    Width = 139
    Height = 54
    Caption = 'RUN'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -40
    Font.Name = 'Times New Roman MT Extra Bold'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    OnClick = ButtonRunClick
  end
  object Memo1: TMemo
    Left = 8
    Top = 8
    Width = 804
    Height = 729
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -40
    Font.Name = 'Courier New'
    Font.Style = [fsBold]
    ImeName = #32043#20809#25340#38899#36755#20837#27861
    Lines.Strings = (
      '***** PL/0 Compiler Demo *****')
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 1
  end
  object EditName: TEdit
    Left = 831
    Top = 74
    Width = 162
    Height = 54
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -40
    Font.Name = 'Courier New'
    Font.Style = [fsBold]
    ImeName = #32043#20809#25340#38899#36755#20837#27861
    ParentFont = False
    TabOrder = 2
    Text = 'E01'
  end
  object ListSwitch: TRadioGroup
    Left = 824
    Top = 168
    Width = 177
    Height = 163
    Caption = #30446#26631#20195#30721
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -40
    Font.Name = #40657#20307
    Font.Style = []
    ItemIndex = 0
    Items.Strings = (
      #26174#31034
      #19981#26174#31034)
    ParentFont = False
    TabOrder = 3
  end
end
