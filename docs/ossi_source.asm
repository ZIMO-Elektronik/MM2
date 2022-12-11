; Motorola
;{
kMo_NeuPak	equ		0
;kMo_		equ		1	
kMo_Grund	equ		2
;kMo_		equ		3
kMo_Fertig	equ		4


MOTOROLA:
	; cMoFlags:		wird bei jedem Stromausfall gelöscht
	;	0: neues Paket zur Weiterverarbeitung vorhanden
	;	1: Bit:0=kurz (0-37ys), 1=lang (170-204ys)
	;	2: neues Bit aus NMRA_CHK
	;	3: erstes Paket Richtungsumkehr für Programmiermodus?
	;	4: Paket bereits ausgeführt
	;	5: 1=Paket gültig
	;	6: neue Pause aus NMRA_CHK
	;	7: Grundpegel an Ltg.2

	; Bit:      7  6  5  4  3  2  1  0
	; cMoDat1:  x  x  x  8  6  4  2  0		Adresse(0-7) u. F0(8-9)
	; cMoDat2:  x  x  x  9  7  5  3  1		 0,0=0 1,1=1 1,0=2   Wertigkeiten der Trits: 27,9,3,1
	; cMoDat3: 17 15 13 11 16 14 12 10		Geschwindigkeit(B0-B3), Richtung,F1-F4(B4-B7)

			bcf		cMoFlags,2			; Flag löschen (neues Bit)
			; wenn letzter Pegel = Grundpegel -> ENDE
			movf	cNmraFlags,w
			xorwf	cMoFlags,w
			andlw	10000000B
			skpnz
			return

MOTOROLA_2:
			incf	cMoCnt,f

			; richtiges Byte wählen
			movlw	18
			cpfslt	cMoCnt				; > 17 ? (Fehler)
			return
			lfsr	0,cMoDat3
			movlw	10
			cpfslt	cMoCnt				; >= 10 ? (Geschw.,Ri,F1-F4)
			bra		Mo_Dat2				; Ja ->
			decf	FSR0L				; ->cMoDat2
			btfss	cMoCnt,0			; Gerade Zahl?
			decf	FSR0L				; Ja, ->cMoDat1
Mo_Dat1:	; Zeiger jetzt auf Zielregister
			; richtiges Bit wählen
			movf	cMoCnt,w			; Cnt->W (0-9)
			andlw	00001110B			; Bit 0 ausmaskieren (0-8) (entspricht Bit 0-4)
			call	BIT_TAB				; Bit in W
			bra		Mo_Dat10
Mo_Dat2:
			movlw	HIGH MO_SPEED_TAB
			movwf	PCLATH
			rlncf	cMoCnt,w			; Cnt*2->W (20-34)
			addlw	-20					; -20 (0-14)
			call	MO_SPEED_TAB
Mo_Dat10:	; Jetzt Maske für Bit in W
			movwf	cIWork1				; zwischenspeichern für Änderung bei Falsch
			andwf	INDF0,w				; Gesuchtes Bit ausmaskieren
			movlw	11111111B			; Wenn Bit=1 dann W=0x80
			skpnz
			movlw	0
			xorwf	cMoFlags,w		; Vergleiche geholtes Bit mit aktuellem Bit -> Zero
			andlw	00000010B			; Ergebnis ausmaskieren
;			skpnz						; Gleich ?
			bz		Mo_Bit_gleich		; Ja ->
Mo_Bit_nicht_gleich:
			bcf		cMoFlags,5			; Paket ungültig
			bcf		cMoFlags,4			; Bit "Gültiges Paket bereits weitergegeben" löschen
			movf	cIWork1,w			; aktuelles Bit holen
			xorwf	INDF0,f				; und richtigstellen
Mo_Bit_gleich:
			movlw	17
			cpfseq	cMoCnt				; Letztes Bit?
			return						; Nein, Fertig ->


			btfss	cMoFlags,5			; Paket gültig?
			return						; Nein, jetzt kommt wieder 2. Paket

		movlb	3
            incf	cEmpfCnt1,f			; Empfangene Digital-Pakete, wird alle 100ms ausgewertet
            clrf	cAC_Entry_Cnt		; 10*60ms=0,6s bevor in AC-Betrieb geht
		movlb	0
            bsf		cFlags2,kNewAbc		; Neue HLU-Daten (damit bei Wechsel von Märklin Bremsstrecke -> Motorola Lok wegfährt)
            movlw	8
            movwf	cDCLim				; Break on DC Begrenzung aus
            
			btfsc	cMoFlags,4			; Paket bereits ausgeführt?
			return						; Ja ->

			movff	cMoDat1,cMoDatC1
			movff	cMoDat2,cMoDatC2
			movff	cMoDat3,cMoDatC3	; Motorola-Paket für Auswertung buffern

			bsf		cMoFlags,4			; Mo-Paket weitergegeben
			bsf		cMoFlags,0			; Neues gültiges Mo-Paket

			return						; Ja, Fertig ->

;-------------------

MO_PAKET:
			bcf		cMoFlags,0			; Empfang bestätigen
            clrf    cSigOffLen2         ; DC-TimeOut löschen
            
cMoDatW1	equ	cWork10
cMoDatW2	equ	cWork11
cMoDatW3	equ	cWork12

  			movff	cMoDatC1,cMoDatW1
			movff	cMoDatC2,cMoDatW2
			movff	cMoDatC3,cMoDatW3

			clrf	cWork				; Adresszähler
			movf	cMoDatW1,w
			iorwf	cMoDatW2,w
			andlw	00000001B			; 1. Trit 0 ?
			bz		TRIT2
			incf	cWork,f				; 1x1 in Zähler
			btfss	cMoDatW2,0			; Trit=2?
			rlncf	cWork,f				; Ja, 2x1 in Zähler
TRIT2:
			movf	cMoDatW1,w
			iorwf	cMoDatW2,w
			andlw	00000010B			; 2. Trit 0 ?
			bz		TRIT3
			movlw	3
			btfss	cMoDatW2,1			; Trit=2?
			movlw	6
			addwf	cWork,f
TRIT3:
			movf	cMoDatW1,w
			iorwf	cMoDatW2,w
			andlw	00000100B			; 2. Trit 0 ?
			bz		TRIT4
			movlw	9
			btfss	cMoDatW2,2			; Trit=2?
			movlw	18
			addwf	cWork,f
TRIT4:
			movf	cMoDatW1,w
			iorwf	cMoDatW2,w
			andlw	00001000B			; 2. Trit 0 ?
			bz		MO_ADRESS
			movlw	27
			btfss	cMoDatW2,3			; Trit=2?
			movlw	54
			addwf	cWork,f
MO_ADRESS:
			btfsc	cFlags2,kMProg		; CV-Prog-Modus aktiv?
			bra		MO_CV_PROG			; Ja ->
			movlw	80
			subwf	cWork,w
			skpnz
			return						; Idle-Mode

			btfss	cMoFlags,3			; Erstes Paket?
			bra		Mo_Erstes_Paket		; Erstes Paket auf Prog-Modus prüfen
Mo_NoProg:
			movlw	80
			tstfsz	cWork				; Adresse = 0 ?
			movf	cWork,w				; Nein, Adr. beibehalten
			movwf	cWork				; Ja, 80 verwenden

			clrf	cWork3
			movlw   11011011B           ; Operationsmode !
            andwf   cSpeedFlags,f       ; ConsistAdr,ConsistCmd,ConsistDir,BroadcastAdr Bits löschen
            
			btfss	cFlags19,0			; Consistadresse da?
            bra		Consist_off			; Nein ->
			movf	cWork,w
			cpfseq	cLokConsAdr			; Empf. Adresse == Consistadr ?
			bra		Consist_off			; Nein, Daten sind nicht an mich gerichtet
			tstfsz	cLokConsAH			; Meine Consist-Adr. > 255?
			bra		Consist_off			; Ja, Daten sind nicht an mich gerichtet
			bsf     cSpeedFlags,5       ; Ja, ConsistCmd aktiv Bit setzen
			bra     MO_AnMich			; Ja, Daten in Arbeitsbereich kopieren

Consist_off:
			btfsc	cCV29Modus,5        ; Kurze Adresse aktiv ?
            return						; Nein, lange Adressen gibts nicht in MOT ->

			movf	cWork,w
			xorwf   cLokAdress,w        ; Vergleiche Adresse mit Lokadresse
			bz		MO_AnMich

			lfsr	0,cCV154
			btfsc	INDF0,3				; Folgeadresse für Lichter aktiv? (cCV154,3)
			return						; Nein

			btfss	cFlags,kMOTaktiv	; MOT war schon da?
			return						; Nein, zuerst muß Hauptadresse gekommen sein bevor Zweitadresse akzeptiert wird
			bsf		cWork3,0			; Folgeadresse (für Lichter)

			decf	cWork,w
			xorwf   cLokAdress,w        ; Vergleiche Adresse-1 mit Lokadresse
			skpz
			return						; nicht meine Adresse

			; Paket an mich
MO_AnMich:	
		movlb	0
			bsf		cFlags,kMOTaktiv	; Motorola am Gleis
			movlw	254
			movwf	cFormat				; MOT Mode

			bcf		cFlags,kDCCaktiv	; wenn Mot an Mich da dann DCC aus
			bsf     PIE1,TMR2IE         ; Timer 2 Interrupt wieder freigeben
            bsf     cSpeedFlags,6       ; Ja, Fahrbefehle freigeben

			btfsc	cWork3,0			; Folgeadresse?
			bra		MO_Zweit			; Ja, F0 nicht auswerten ->

			movff	cFuncInA1,cLokInstr1
			bcf		cLokInstr1,4		; F0 mal aus
 			btfsc	cMoDatW1,4
 			bsf		cLokInstr1,4		; F0 ein
			call	Func_Grp_1

MO_Zweit:
			swapf	cMoDatW3,w
			xorwf	cMoDatW3,w
			andlw	00001111B			; Altes Format?
			bnz		MO_NEU
MO_ALT:
			movf	cMoDatW3,w
			andlw	00001111B
			xorlw	1					; SP=1 -> Richtungsumkehr
			bz		MO_RIUM_ALT
			bcf		cFlags4,kkeinriwe	; Jetzt werden RiUm-Befehle wieder angenommen
MO_SPEED_ALT:
			clrf	PRODL
			xorlw	1					; wieder richtigstellen
			bz		MO_SPEED_ALT_NULL	; Speed=0 ->
			addlw	-1					; 15=FS14
			mullw	18					; auf 252 Stufen
MO_SPEED_ALT_NULL:
			call	Test_Adress_MO_2
			btfss	WREG,4				; 0=Ausführen
			bra		MO_SPEED_END		; Befehl abarbeiten
			return						; Befehl jetzt nicht erlaubt

MO_RIUM_ALT:
			btfsc	cFlags4,kkeinriwe
			return
			bsf		cFlags4,kkeinriwe	; Jetzt werden keine RiUm-Befehle mehr angenommen

			setf	cWork				; Geschwindigkeit vortäuschen
			call	Test_Consist		; Consist-Auto Übernahme durch Richtung falls aktiv
			btfsc	WREG,4				; 0=Ausführen
			return

			btg		cSpeedFlags,0
			clrf	PRODL
			bra		MO_SPEED_END

MO_NEU:
			movff	cMoDatW3,cWork2		; zwischenspeichern für Ausnahmen

			swapf	cMoDatW3,w
			andlw	00000111B
			addlw	-2					; Vorw.?
			bz		MO_VWRW				; MO_VORW
			addlw	-3					; Rückw.?
			bz		MO_VWRW				; MO_RÜCKW
MO_FUNC:
			swapf	cMoDatW3,w
			movwf	cWork2
MO_FUNC_2:	movlw	HIGH MO_FUNC_TAB	; Einsprung Ausnahmen
			movwf	PCLATH
			rlncf	cWork2,w			; *2 wegen PIC18
			andlw	00001110B			; Funktion ausmaskieren
			call	MO_FUNC_TAB
			btfsc	cWork3,0			; Zweitadresse?
			bra		MO_F2				; Ja ->

			movff	cFuncInA1,cLokInstr1
			iorwf	cLokInstr1,f
			xorlw	0xFF				; Maske invertieren
			btfss	cMoDatW3,7			; Funktion ein ?
			andwf	cLokInstr1,f		; Nein, aus
			bcf		cLokInstr1,4		; F0 mal aus
 			btfsc	cMoDatW1,4
 			bsf		cLokInstr1,4		; F0 ein
			call	Func_Grp_1

		; jetzt Speed bearbeiten
MO_SPEED_NEU:
			clrf	PRODL
			movf	cMoDatW3,w
			andlw	00001111B			; Speed ausmaskieren
			bz		MO_SPEED_NEU_NULL	; Speed=0 ->
			addlw	-1
			mullw	18					; auf 252 Stufen
MO_SPEED_NEU_NULL:
			call	Test_Adress_MO_2
			btfsc	WREG,4				; 0=Ausführen
			return						; Befehl jetzt nicht erlaubt

MO_SPEED_END:
			movff	PRODL,cInpSpTmp
			btfss	cWork3,1			; Richtung auswerten?
			bra		MO_SP_NO_RI
			bsf		cSpeedFlags,0		; nach vorwärts
			btfsc	cMoDatW3,4
			bcf		cSpeedFlags,0		; nach rückwärts
MO_SP_NO_RI:
			movf	cSpeedFlags,w
			andlw	00000011B
			bz		MO_SPEED_E2			; Soll-Ist-Richtung gleich
			xorlw	00000011B
			bz		MO_SPEED_E2
			; Wenn Richtungswechsel während Fahrt dann Notstop
			clrf	cInpSpeedH
			clrf	cInpSpeed
			clrf	cOutSpeedH
			clrf	cOutSpeed
MO_SPEED_E2:
			goto	CALC_SPEED

		; Richtung bearbeiten
MO_VWRW:
			swapf	cMoDatW3,w
			xorwf	cMoDatW3,w
			andlw	10000000B			; H=D?
			bz		MO_FUNC_2			; Ja, -> Ausnahmen

			btfss	cFlags6,Sperr_Richt	; Abgleichfahrt?
			bsf		cWork3,1			; Nein, Richtung übernehmen
			bra		MO_SPEED_NEU

; ---------------------------------------------------------------------
Test_Adress_MO_2:
			; Testen ob Richtung/Speed ausgewertet werden soll
			; WREG,4=1 -> nicht auswerten
			; WREG,4=0 -> auswerten
			bsf		WREG,4				; 1=Nicht Ausführen
			btfsc	cWork3,0			; Folgeadresse?
			return						; Ja, nicht auswerten

			movff	PRODL,cWork
			call	Test_Consist
			return						; ConsistAdr aktiv und ConsistCmd, 1, auswerten

; ---------------------------------------------------------------------

MO_F2:
  			mullw	16
  			movf	PRODL,w
			iorwf	cFuncInA1,f
			xorlw	0xFF				; Maske invertieren
			btfss	cMoDatW3,7			; Funktion ein ?
			andwf	cFuncInA1,f			; Nein, aus
			return



;**********	Motorola Programmiermodus ***********
Mo_Erstes_Paket:
			bsf		cMoFlags,3			; jetzt nicht mehr erstes Paket
			movf	cMoDatW3,w
			andlw	00001111B			; alte Geschwindigkeitsinformation ausmaskieren
			xorlw	1					; Richtungsumkehr?
			skpz
			bra		Mo_NoProg			; Nein, Befehl normal abarbeiten ->
			
			movf	cWork,w
			xorwf   cLokAdress,w        ; Vergleiche Adresse mit Lokadresse
			bz		Mo_Prog1			; Ja, CV-Prog starten ->
			
			tstfsz	cWork				; Adresse=0?
			bra		Mo_NoProg			; Nein, Befehl normal abarbeiten ->
			
Mo_Prog1:
			bsf		cFlags2,kMProg		; Prog.Modus ein
			bcf		cFlags4,kkeinriwe	; Richrungswechsel erst gültig nachdem ein nicht-RiWe.Befehl kam
			clrf	cFuncOut
			bsf		cFuncAktiv1,FA0V_PIN
		movlb	1
			movf	cWork,w
			movwf	cMoPAdr				; Adresse merken
			movlw	10
			movwf	cMoPWork			; Zähler
			movlw	1
			movwf	cMoPWork2			; Step 1-4
		movlb	0
			bra		Waitdannweiter

;**********	Motorola Programmiermodus ***********
MO_CV_PROG:	; Adr. in cWork
            call    ENABLE_PRG_1        ; Zugriff auf EEprom erlauben
			movf	cMoDatW3,w
			andlw	00001111B			; alte Geschwindigkeitsinformation ausmaskieren
			xorlw	1					; Richtungsumkehr?
			bnz		MO_kein_rium		; Nein ->
			; RiWe
		movlb	1
			movf	cMoPAdr,w			
		movlb	0
			cpfseq	cWork				; akt. Adr = letzte Adr.(auf der ein RiWe kam)
			bra		M_PROG_00			; Nein, gültiger neuer RiWe
			btfss	cFlags4,kkeinriwe	; Sperre für diese Adresse aktiv?
			return						; Ja ->
M_PROG_00:
			movf	cWork,w				; Adr. in W
		movlb	1
			movwf	cMoPAdr				; Adresse auf der der RiWe kam merken
		movlb	0
			bcf		cFlags4,kkeinriwe	; Sperre für diese Adresse aktiv!

			xorlw	80					; 80=0
			skpnz
			movwf	cWork				; 0 übernehmen

		movlb	1
			movlw	10
			movwf	cMoPWork			; Zähler zurücksetzen

			incf	cMoPWork2,f
			decf	cMoPWork2,w
			skpnz
			return						; erster Einsprung
		movlb	0
			addlw	-1
			bz		M_PROG_10
			addlw	-1
			bz		M_PROG_20
			addlw	-1
			bz		M_PROG_30
		; 4. Umpolung
M_PROG_40:	movf	cWork,w
			addwf	cCvDat3
		movlb	1
			movlw	1
			movwf	cMoPWork2			; Step 1-4
			movlw	10
			movwf	cMoPWork			; Zähler zurücksetzen
		movlb	0
			bcf		FA0V_PORT,FA0V_PIN		; FA0v aus

			incf	cCvDat2,w
			bz		MoCvModusUm
			movff	cCvDat1,EEADRH		; CV-Nr. MSB übergeben
			call	CHK_WR_ADR			; Wenn Zero gesetzt dann schreiben nicht erlaubt
			bz		Waitdannweiter

            movf    cCvDat2,w           ; CV Adresse holen 1->0
            movwf   cEEPromAdr          ;
            call    ENABLE_PRG_2       ; Zugriff auf EEprom erlauben
            movf    cCvDat3,w             ; ver"ndertes CV Datenbyte holen
            call    I2C_TX_BLK         ; Ins EEprom schreiben
            clrf	EEADRH
            bsf		INTCON,GIEH
			bsf		FA0V_PORT,FA0V_PIN
			bra		Waitdannweiter

M_PROG_10:	; 1. Umpolung
			clrf	cCvDat3
			btfss	cFlags4,kmocvlang
			bra		M_PROG_15			; kurz ->
			movlw	10
			mulwf	cWork
			movff	PRODL,cCvDat2		; Adr. x 10 LSB in cCVDat2
			movff	PRODH,cCvDat1		; Adr. x 10 MSB in cCVDat1
			bra		Waitdannweiter
M_PROG_15:	; 1. Kurzform
			decf	cWork,w
			movwf	cCvDat2
			bra		Waitdannweiter
M_PROG_20:	; 2. Umpolung
			btfss	cFlags4,kmocvlang
			bra		M_PROG_40			; kurz, wie 4. bei lang bearbeiten ->
			decf	cWork,w
			addwf	cCvDat2,f
			movlw	0
			addwfc	cCvDat1,f
			bra		Waitdannweiter

M_PROG_30:	; 3. Umpolung
			movlw	10
			mulwf	cWork
			movff	PRODL,cCvDat3		; Adr. x 10 in cCVDat3
			bra		Waitdannweiter


MO_kein_rium:	; Adr. in cWork
		movlb	1
			movf	cMoPAdr,w			
		movlb	0
			cpfseq	cWork				; akt. Adr = letzte Adr.(auf der ein RiWe kam)
			return						; Nein ->
			bsf		cFlags4,kkeinriwe	; Ja, Sperre (für diese Adresse) lösen
			return
MoCvModusUm:
			movlw	2
			cpfseq	cCvDat3
			bra		MoCvModusUm2
			; CV0=2 ist CV-Modus beenden
			clrf	cCheck1
			reset
MoCvModusUm2:	; CV0=0 -> auf langen Modus, alles andere(auáer 2) ist auf kurz zurück
			bsf		cFlags4,kmocvlang	; mal auf lang setzen
			tstfsz	cCvDat3				; Wert = 0 ?
			bcf		cFlags4,kmocvlang	; Nein, auf kurz setzen
			bsf		FA0V_PORT,FA0V_PIN
			return						; Fertig ->

Waitdannweiter:
		return
			movlw	20
			movwf	cWork3
MWait3:		clrf	cWork2
MWait2:		clrf	cWork
MWait1:		clrwdt
			decfsz	cWork
			bra		MWait1
			decfsz	cWork2
			bra		MWait2
			decfsz	cWork3
			bra		MWait3
			bcf		FA0V_PORT,FA0V_PIN
			return

;**********	Motorola Programmiermodus ***********

MPROG_BLI:
		movlb	1
			btg		cMoPWork3,0
			btfss	cMoPWork3,0
			return
			rlncf	cMoPWork2,w			; Step*2 in W 2-8
			addlw	1					; +1 = 3-9
			subwf	cMoPWork,w
			skpnc
			bra		MPBLI_END

			lfsr	0,FA0V_FL			; cFuncLat1
			movlw	0
			btfsc	INDF0,FA0V_PIN
			movlw	1
			xorwf	cMoPWork,w			; =
			andlw	00000001B			; cMoPWork,0 ?
			skpnz
			btg		INDF0,FA0V_PIN		; Ja

MPBLI_END:
			decfsz	cMoPWork,f
			return
			movlw	10
			movwf	cMoPWork			; Zählt 10-1
			return
;}
