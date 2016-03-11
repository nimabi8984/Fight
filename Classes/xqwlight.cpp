//
//  xqwlight.cpp
//  Fight
//
//  Created by CT on 3/11/16.
//
//

#include "xqwlight.hpp"
/**
 * XiangQi Wizard Light - A Very Simple Chinese Chess Program
 * Designed by Morning Yellow, Version: 0.6, Last Modified: Mar. 2008
 * Copyright (C) 2004-2008 www.xqbase.com
 *
 * 象棋小巫师 0.6 的目标：
 * 一、实现开局库；
 * 二、实现PVS(主要变例搜索)；
 * 三、把根节点的搜索单独处理，增加搜索的随机性；
 * 四、克服由长将引起的置换表的不稳定性。
 */

// 初始化棋盘
void PositionStruct::Startup(void) {
    int sq, pc;
    ClearBoard();
    for (sq = 0; sq < 256; sq ++) {
        pc = cucpcStartup[sq];
        if (pc != 0) {
            AddPiece(sq, pc);
        }
    }
    SetIrrev();
}

// 搬一步棋的棋子
int PositionStruct::MovePiece(int mv) {
    int sqSrc, sqDst, pc, pcCaptured;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pcCaptured = ucpcSquares[sqDst];
    if (pcCaptured != 0) {
        DelPiece(sqDst, pcCaptured);
    }
    pc = ucpcSquares[sqSrc];
    DelPiece(sqSrc, pc);
    AddPiece(sqDst, pc);
    return pcCaptured;
}

// 撤消搬一步棋的棋子
void PositionStruct::UndoMovePiece(int mv, int pcCaptured) {
    int sqSrc, sqDst, pc;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pc = ucpcSquares[sqDst];
    DelPiece(sqDst, pc);
    AddPiece(sqSrc, pc);
    if (pcCaptured != 0) {
        AddPiece(sqDst, pcCaptured);
    }
}

// 走一步棋
BOOL PositionStruct::MakeMove(int mv) {
    int pcCaptured;
    DWORD dwKey;
    
    dwKey = zobr.dwKey;
    pcCaptured = MovePiece(mv);
    if (Checked()) {
        UndoMovePiece(mv, pcCaptured);
        return false;
    }
    ChangeSide();
    mvsList[nMoveNum].Set(mv, pcCaptured, Checked(), dwKey);
    nMoveNum ++;
    nDistance ++;
    return true;
}


// 生成所有走法，如果"bCapture"为"TRUE"则只生成吃子走法
int PositionStruct::GenerateMoves(int *mvs, BOOL bCapture) const {
    int i, j, nGenMoves, nDelta, sqSrc, sqDst;
    int pcSelfSide, pcOppSide, pcSrc, pcDst;
    // 生成所有走法，需要经过以下几个步骤：
    
    nGenMoves = 0;
    pcSelfSide = SIDE_TAG(sdPlayer);
    pcOppSide = OPP_SIDE_TAG(sdPlayer);
    for (sqSrc = 0; sqSrc < 256; sqSrc ++) {
        
        // 1. 找到一个本方棋子，再做以下判断：
        pcSrc = ucpcSquares[sqSrc];
        if ((pcSrc & pcSelfSide) == 0) {
            continue;
        }
        
        // 2. 根据棋子确定走法
        switch (pcSrc - pcSelfSide) {
            case PIECE_KING:
                for (i = 0; i < 4; i ++) {
                    sqDst = sqSrc + ccKingDelta[i];
                    if (!IN_FORT(sqDst)) {
                        continue;
                    }
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves ++;
                    }
                }
                break;
            case PIECE_ADVISOR:
                for (i = 0; i < 4; i ++) {
                    sqDst = sqSrc + ccAdvisorDelta[i];
                    if (!IN_FORT(sqDst)) {
                        continue;
                    }
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves ++;
                    }
                }
                break;
            case PIECE_BISHOP:
                for (i = 0; i < 4; i ++) {
                    sqDst = sqSrc + ccAdvisorDelta[i];
                    if (!(IN_BOARD(sqDst) && HOME_HALF(sqDst, sdPlayer) && ucpcSquares[sqDst] == 0)) {
                        continue;
                    }
                    sqDst += ccAdvisorDelta[i];
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves ++;
                    }
                }
                break;
            case PIECE_KNIGHT:
                for (i = 0; i < 4; i ++) {
                    sqDst = sqSrc + ccKingDelta[i];
                    if (ucpcSquares[sqDst] != 0) {
                        continue;
                    }
                    for (j = 0; j < 2; j ++) {
                        sqDst = sqSrc + ccKnightDelta[i][j];
                        if (!IN_BOARD(sqDst)) {
                            continue;
                        }
                        pcDst = ucpcSquares[sqDst];
                        if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                            mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                            nGenMoves ++;
                        }
                    }
                }
                break;
            case PIECE_ROOK:
                for (i = 0; i < 4; i ++) {
                    nDelta = ccKingDelta[i];
                    sqDst = sqSrc + nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst == 0) {
                            if (!bCapture) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves ++;
                            }
                        } else {
                            if ((pcDst & pcOppSide) != 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves ++;
                            }
                            break;
                        }
                        sqDst += nDelta;
                    }
                }
                break;
            case PIECE_CANNON:
                for (i = 0; i < 4; i ++) {
                    nDelta = ccKingDelta[i];
                    sqDst = sqSrc + nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst == 0) {
                            if (!bCapture) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves ++;
                            }
                        } else {
                            break;
                        }
                        sqDst += nDelta;
                    }
                    sqDst += nDelta;
                    while (IN_BOARD(sqDst)) {
                        pcDst = ucpcSquares[sqDst];
                        if (pcDst != 0) {
                            if ((pcDst & pcOppSide) != 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves ++;
                            }
                            break;
                        }
                        sqDst += nDelta;
                    }
                }
                break;
            case PIECE_PAWN:
                sqDst = SQUARE_FORWARD(sqSrc, sdPlayer);
                if (IN_BOARD(sqDst)) {
                    pcDst = ucpcSquares[sqDst];
                    if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                        mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                        nGenMoves ++;
                    }
                }
                if (AWAY_HALF(sqSrc, sdPlayer)) {
                    for (nDelta = -1; nDelta <= 1; nDelta += 2) {
                        sqDst = sqSrc + nDelta;
                        if (IN_BOARD(sqDst)) {
                            pcDst = ucpcSquares[sqDst];
                            if (bCapture ? (pcDst & pcOppSide) != 0 : (pcDst & pcSelfSide) == 0) {
                                mvs[nGenMoves] = MOVE(sqSrc, sqDst);
                                nGenMoves ++;
                            }
                        }
                    }
                }
                break;
        }
    }
    return nGenMoves;
}

// 判断走法是否合理
BOOL PositionStruct::LegalMove(int mv) const {
    int sqSrc, sqDst, sqPin;
    int pcSelfSide, pcSrc, pcDst, nDelta;
    // 判断走法是否合法，需要经过以下的判断过程：
    
    // 1. 判断起始格是否有自己的棋子
    sqSrc = SRC(mv);
    pcSrc = ucpcSquares[sqSrc];
    pcSelfSide = SIDE_TAG(sdPlayer);
    if ((pcSrc & pcSelfSide) == 0) {
        return false;
    }
    
    // 2. 判断目标格是否有自己的棋子
    sqDst = DST(mv);
    pcDst = ucpcSquares[sqDst];
    if ((pcDst & pcSelfSide) != 0) {
        return false;
    }
    
    // 3. 根据棋子的类型检查走法是否合理
    switch (pcSrc - pcSelfSide) {
        case PIECE_KING:
            return IN_FORT(sqDst) && KING_SPAN(sqSrc, sqDst);
        case PIECE_ADVISOR:
            return IN_FORT(sqDst) && ADVISOR_SPAN(sqSrc, sqDst);
        case PIECE_BISHOP:
            return SAME_HALF(sqSrc, sqDst) && BISHOP_SPAN(sqSrc, sqDst) &&
            ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0;
        case PIECE_KNIGHT:
            sqPin = KNIGHT_PIN(sqSrc, sqDst);
            return sqPin != sqSrc && ucpcSquares[sqPin] == 0;
        case PIECE_ROOK:
        case PIECE_CANNON:
            if (SAME_RANK(sqSrc, sqDst)) {
                nDelta = (sqDst < sqSrc ? -1 : 1);
            } else if (SAME_FILE(sqSrc, sqDst)) {
                nDelta = (sqDst < sqSrc ? -16 : 16);
            } else {
                return false;
            }
            sqPin = sqSrc + nDelta;
            while (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                sqPin += nDelta;
            }
            if (sqPin == sqDst) {
                return pcDst == 0 || pcSrc - pcSelfSide == PIECE_ROOK;
            } else if (pcDst != 0 && pcSrc - pcSelfSide == PIECE_CANNON) {
                sqPin += nDelta;
                while (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                    sqPin += nDelta;
                }
                return sqPin == sqDst;
            } else {
                return false;
            }
        case PIECE_PAWN:
            if (AWAY_HALF(sqDst, sdPlayer) && (sqDst == sqSrc - 1 || sqDst == sqSrc + 1)) {
                return true;
            }
            return sqDst == SQUARE_FORWARD(sqSrc, sdPlayer);
        default:
            return false;
    }
}

// 判断是否被将军
BOOL PositionStruct::Checked() const {
    int i, j, sqSrc, sqDst;
    int pcSelfSide, pcOppSide, pcDst, nDelta;
    pcSelfSide = SIDE_TAG(sdPlayer);
    pcOppSide = OPP_SIDE_TAG(sdPlayer);
    // 找到棋盘上的帅(将)，再做以下判断：
    
    for (sqSrc = 0; sqSrc < 256; sqSrc ++) {
        if (ucpcSquares[sqSrc] != pcSelfSide + PIECE_KING) {
            continue;
        }
        
        // 1. 判断是否被对方的兵(卒)将军
        if (ucpcSquares[SQUARE_FORWARD(sqSrc, sdPlayer)] == pcOppSide + PIECE_PAWN) {
            return true;
        }
        for (nDelta = -1; nDelta <= 1; nDelta += 2) {
            if (ucpcSquares[sqSrc + nDelta] == pcOppSide + PIECE_PAWN) {
                return true;
            }
        }
        
        // 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
        for (i = 0; i < 4; i ++) {
            if (ucpcSquares[sqSrc + ccAdvisorDelta[i]] != 0) {
                continue;
            }
            for (j = 0; j < 2; j ++) {
                pcDst = ucpcSquares[sqSrc + ccKnightCheckDelta[i][j]];
                if (pcDst == pcOppSide + PIECE_KNIGHT) {
                    return true;
                }
            }
        }
        
        // 3. 判断是否被对方的车或炮将军(包括将帅对脸)
        for (i = 0; i < 4; i ++) {
            nDelta = ccKingDelta[i];
            sqDst = sqSrc + nDelta;
            while (IN_BOARD(sqDst)) {
                pcDst = ucpcSquares[sqDst];
                if (pcDst != 0) {
                    if (pcDst == pcOppSide + PIECE_ROOK || pcDst == pcOppSide + PIECE_KING) {
                        return true;
                    }
                    break;
                }
                sqDst += nDelta;
            }
            sqDst += nDelta;
            while (IN_BOARD(sqDst)) {
                int pcDst = ucpcSquares[sqDst];
                if (pcDst != 0) {
                    if (pcDst == pcOppSide + PIECE_CANNON) {
                        return true;
                    }
                    break;
                }
                sqDst += nDelta;
            }
        }
        return false;
    }
    return false;
}

// 判断是否被杀
BOOL PositionStruct::IsMate(void) {
    int i, nGenMoveNum, pcCaptured;
    int mvs[MAX_GEN_MOVES];
    
    nGenMoveNum = GenerateMoves(mvs);
    for (i = 0; i < nGenMoveNum; i ++) {
        pcCaptured = MovePiece(mvs[i]);
        if (!Checked()) {
            UndoMovePiece(mvs[i], pcCaptured);
            return false;
        } else {
            UndoMovePiece(mvs[i], pcCaptured);
        }
    }
    return true;
}

// 检测重复局面
int PositionStruct::RepStatus(int nRecur) const {
    BOOL bSelfSide, bPerpCheck, bOppPerpCheck;
    const MoveStruct *lpmvs;
    
    bSelfSide = false;
    bPerpCheck = bOppPerpCheck = true;
    lpmvs = mvsList + nMoveNum - 1;
    while (lpmvs->wmv != 0 && lpmvs->ucpcCaptured == 0) {
        if (bSelfSide) {
            bPerpCheck = bPerpCheck && lpmvs->ucbCheck;
            if (lpmvs->dwKey == zobr.dwKey) {
                nRecur --;
                if (nRecur == 0) {
                    return 1 + (bPerpCheck ? 2 : 0) + (bOppPerpCheck ? 4 : 0);
                }
            }
        } else {
            bOppPerpCheck = bOppPerpCheck && lpmvs->ucbCheck;
        }
        bSelfSide = !bSelfSide;
        lpmvs --;
    }
    return 0;
}

// 对局面镜像
void PositionStruct::Mirror(PositionStruct &posMirror) const {
    int sq, pc;
    posMirror.ClearBoard();
    for (sq = 0; sq < 256; sq ++) {
        pc = ucpcSquares[sq];
        if (pc != 0) {
            posMirror.AddPiece(MIRROR_SQUARE(sq), pc);
        }
    }
    if (sdPlayer == 1) {
        posMirror.ChangeSide();
    }
    posMirror.SetIrrev();
}

// 用空密钥初始化密码流生成器
void RC4Struct::InitZero(void) {
    int i, j;
    unsigned char uc;
    
    x = y = j = 0;
    for (i = 0; i < 256; i ++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i ++) {
        j = (j + s[i]) & 255;
        uc = s[i];
        s[i] = s[j];
        s[j] = uc;
    }
}

// 得到下一个走法
int SortStruct::Next(void) {
    int mv;
    switch (nPhase) {
            // "nPhase"表示着法启发的若干阶段，依次为：
            
            // 0. 置换表着法启发，完成后立即进入下一阶段；
        case PHASE_HASH:
            nPhase = PHASE_KILLER_1;
            if (mvHash != 0) {
                return mvHash;
            }
            // 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同
            
            // 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
        case PHASE_KILLER_1:
            nPhase = PHASE_KILLER_2;
            if (mvKiller1 != mvHash && mvKiller1 != 0 && pos.LegalMove(mvKiller1)) {
                return mvKiller1;
            }
            
            // 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
        case PHASE_KILLER_2:
            nPhase = PHASE_GEN_MOVES;
            if (mvKiller2 != mvHash && mvKiller2 != 0 && pos.LegalMove(mvKiller2)) {
                return mvKiller2;
            }
            
            // 3. 生成所有着法，完成后立即进入下一阶段；
        case PHASE_GEN_MOVES:
            nPhase = PHASE_REST;
            nGenMoves = pos.GenerateMoves(mvs);
            qsort(mvs, nGenMoves, sizeof(int), CompareHistory);
            nIndex = 0;
            
            // 4. 对剩余着法做历史表启发；
        case PHASE_REST:
            while (nIndex < nGenMoves) {
                mv = mvs[nIndex];
                nIndex ++;
                if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2) {
                    return mv;
                }
            }
            
            // 5. 没有着法了，返回零。
        default:
            return 0;
    }
}

/*
// TransparentBlt 的替代函数，用来修正原函数在 Windows 98 下资源泄漏的问题
static void TransparentBlt2(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
                            HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent) {
    HDC hImageDC, hMaskDC;
    HBITMAP hOldImageBMP, hImageBMP, hOldMaskBMP, hMaskBMP;
    
    hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);
    hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);
    hImageDC = CreateCompatibleDC(hdcDest);
    hMaskDC = CreateCompatibleDC(hdcDest);
    hOldImageBMP = (HBITMAP) SelectObject(hImageDC, hImageBMP);
    hOldMaskBMP = (HBITMAP) SelectObject(hMaskDC, hMaskBMP);
    
    if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc) {
        BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
               hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
    } else {
        StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
                   hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
    }
    SetBkColor(hImageDC, crTransparent);
    BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);
    SetBkColor(hImageDC, RGB(0,0,0));
    SetTextColor(hImageDC, RGB(255,255,255));
    BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);
    SetBkColor(hdcDest, RGB(255,255,255));
    SetTextColor(hdcDest, RGB(0,0,0));
    BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
           hMaskDC, 0, 0, SRCAND);
    BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
           hImageDC, 0, 0, SRCPAINT);
    
    SelectObject(hImageDC, hOldImageBMP);
    DeleteDC(hImageDC);
    SelectObject(hMaskDC, hOldMaskBMP);
    DeleteDC(hMaskDC);
    DeleteObject(hImageBMP);
    DeleteObject(hMaskBMP);
}

// 绘制透明图片
inline void DrawTransBmp(HDC hdc, HDC hdcTmp, int xx, int yy, HBITMAP bmp) {
    SelectObject(hdcTmp, bmp);
    TransparentBlt2(hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, hdcTmp, 0, 0, SQUARE_SIZE, SQUARE_SIZE, MASK_COLOR);
}

// 绘制棋盘
static void DrawBoard(HDC hdc) {
    int x, y, xx, yy, sq, pc;
    HDC hdcTmp;
    
    // 画棋盘
    hdcTmp = CreateCompatibleDC(hdc);
    SelectObject(hdcTmp, Xqwl.bmpBoard);
    BitBlt(hdc, 0, 0, BOARD_WIDTH, BOARD_HEIGHT, hdcTmp, 0, 0, SRCCOPY);
    // 画棋子
    for (x = FILE_LEFT; x <= FILE_RIGHT; x ++) {
        for (y = RANK_TOP; y <= RANK_BOTTOM; y ++) {
            if (Xqwl.bFlipped) {
                xx = BOARD_EDGE + (FILE_FLIP(x) - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE + (RANK_FLIP(y) - RANK_TOP) * SQUARE_SIZE;
            } else {
                xx = BOARD_EDGE + (x - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE + (y - RANK_TOP) * SQUARE_SIZE;
            }
            sq = COORD_XY(x, y);
            pc = pos.ucpcSquares[sq];
            if (pc != 0) {
                DrawTransBmp(hdc, hdcTmp, xx, yy, Xqwl.bmpPieces[pc]);
            }
            if (sq == Xqwl.sqSelected || sq == SRC(Xqwl.mvLast) || sq == DST(Xqwl.mvLast)) {
                DrawTransBmp(hdc, hdcTmp, xx, yy, Xqwl.bmpSelected);
            }
        }
    }
    DeleteDC(hdcTmp);
}

// 播放资源声音
inline void PlayResWav(int nResId) {
    PlaySound(MAKEINTRESOURCE(nResId), Xqwl.hInst, SND_ASYNC | SND_NOWAIT | SND_RESOURCE);
}

// 弹出不带声音的提示框
static void MessageBoxMute(LPCSTR lpszText) {
    MSGBOXPARAMS mbp;
    mbp.cbSize = sizeof(MSGBOXPARAMS);
    mbp.hwndOwner = Xqwl.hWnd;
    mbp.hInstance = NULL;
    mbp.lpszText = lpszText;
    mbp.lpszCaption = "象棋小巫师";
    mbp.dwStyle = MB_USERICON;
    mbp.lpszIcon = MAKEINTRESOURCE(IDI_INFORMATION);
    mbp.dwContextHelpId = 0;
    mbp.lpfnMsgBoxCallback = NULL;
    mbp.dwLanguageId = 0;
    if (MessageBoxIndirect(&mbp) == 0) {
        // 系统图标在 Windows 98 下会失败，所以要使用应用程序图标
        mbp.hInstance = Xqwl.hInst;
        mbp.lpszIcon = MAKEINTRESOURCE(IDI_APPICON);
        MessageBoxIndirect(&mbp);
    }
}

// "DrawSquare"参数
const BOOL DRAW_SELECTED = TRUE;

// 绘制格子
static void DrawSquare(int sq, BOOL bSelected = FALSE) {
    int sqFlipped, xx, yy, pc;
    
    sqFlipped = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
    xx = BOARD_EDGE + (FILE_X(sqFlipped) - FILE_LEFT) * SQUARE_SIZE;
    yy = BOARD_EDGE + (RANK_Y(sqFlipped) - RANK_TOP) * SQUARE_SIZE;
    SelectObject(Xqwl.hdcTmp, Xqwl.bmpBoard);
    BitBlt(Xqwl.hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, Xqwl.hdcTmp, xx, yy, SRCCOPY);
    pc = pos.ucpcSquares[sq];
    if (pc != 0) {
        DrawTransBmp(Xqwl.hdc, Xqwl.hdcTmp, xx, yy, Xqwl.bmpPieces[pc]);
    }
    if (bSelected) {
        DrawTransBmp(Xqwl.hdc, Xqwl.hdcTmp, xx, yy, Xqwl.bmpSelected);
    }
}

// 电脑回应一步棋
static void ResponseMove(void) {
    int vlRep;
    // 电脑走一步棋
    SetCursor((HCURSOR) LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
    SearchMain();
    SetCursor((HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
    pos.MakeMove(Search.mvResult);
    // 清除上一步棋的选择标记
    DrawSquare(SRC(Xqwl.mvLast));
    DrawSquare(DST(Xqwl.mvLast));
    // 把电脑走的棋标记出来
    Xqwl.mvLast = Search.mvResult;
    DrawSquare(SRC(Xqwl.mvLast), DRAW_SELECTED);
    DrawSquare(DST(Xqwl.mvLast), DRAW_SELECTED);
    // 检查重复局面
    vlRep = pos.RepStatus(3);
    if (pos.IsMate()) {
        // 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
        PlayResWav(IDR_LOSS);
        MessageBoxMute("请再接再厉！");
        Xqwl.bGameOver = TRUE;
    } else if (vlRep > 0) {
        vlRep = pos.RepValue(vlRep);
        // 注意："vlRep"是对玩家来说的分值
        PlayResWav(vlRep < -WIN_VALUE ? IDR_LOSS : vlRep > WIN_VALUE ? IDR_WIN : IDR_DRAW);
        MessageBoxMute(vlRep < -WIN_VALUE ? "长打作负，请不要气馁！" :
                       vlRep > WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
        Xqwl.bGameOver = TRUE;
    } else if (pos.nMoveNum > 100) {
        PlayResWav(IDR_DRAW);
        MessageBoxMute("超过自然限着作和，辛苦了！");
        Xqwl.bGameOver = TRUE;
    } else {
        // 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
        PlayResWav(pos.InCheck() ? IDR_CHECK2 : pos.Captured() ? IDR_CAPTURE2 : IDR_MOVE2);
        if (pos.Captured()) {
            pos.SetIrrev();
        }
    }
}

// 点击格子事件处理
static void ClickSquare(int sq) {
    int pc, mv, vlRep;
    Xqwl.hdc = GetDC(Xqwl.hWnd);
    Xqwl.hdcTmp = CreateCompatibleDC(Xqwl.hdc);
    sq = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
    pc = pos.ucpcSquares[sq];
    
    if ((pc & SIDE_TAG(pos.sdPlayer)) != 0) {
        // 如果点击自己的子，那么直接选中该子
        if (Xqwl.sqSelected != 0) {
            DrawSquare(Xqwl.sqSelected);
        }
        Xqwl.sqSelected = sq;
        DrawSquare(sq, DRAW_SELECTED);
        if (Xqwl.mvLast != 0) {
            DrawSquare(SRC(Xqwl.mvLast));
            DrawSquare(DST(Xqwl.mvLast));
        }
        PlayResWav(IDR_CLICK); // 播放点击的声音
        
    } else if (Xqwl.sqSelected != 0 && !Xqwl.bGameOver) {
        // 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
        mv = MOVE(Xqwl.sqSelected, sq);
        if (pos.LegalMove(mv)) {
            if (pos.MakeMove(mv)) {
                Xqwl.mvLast = mv;
                DrawSquare(Xqwl.sqSelected, DRAW_SELECTED);
                DrawSquare(sq, DRAW_SELECTED);
                Xqwl.sqSelected = 0;
                // 检查重复局面
                vlRep = pos.RepStatus(3);
                if (pos.IsMate()) {
                    // 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
                    PlayResWav(IDR_WIN);
                    MessageBoxMute("祝贺你取得胜利！");
                    Xqwl.bGameOver = TRUE;
                } else if (vlRep > 0) {
                    vlRep = pos.RepValue(vlRep);
                    // 注意："vlRep"是对电脑来说的分值
                    PlayResWav(vlRep > WIN_VALUE ? IDR_LOSS : vlRep < -WIN_VALUE ? IDR_WIN : IDR_DRAW);
                    MessageBoxMute(vlRep > WIN_VALUE ? "长打作负，请不要气馁！" :
                                   vlRep < -WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
                    Xqwl.bGameOver = TRUE;
                } else if (pos.nMoveNum > 100) {
                    PlayResWav(IDR_DRAW);
                    MessageBoxMute("超过自然限着作和，辛苦了！");
                    Xqwl.bGameOver = TRUE;
                } else {
                    // 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
                    PlayResWav(pos.InCheck() ? IDR_CHECK : pos.Captured() ? IDR_CAPTURE : IDR_MOVE);
                    if (pos.Captured()) {
                        pos.SetIrrev();
                    }
                    ResponseMove(); // 轮到电脑走棋
                }
            } else {
                PlayResWav(IDR_ILLEGAL); // 播放被将军的声音
            }
        }
        // 如果根本就不符合走法(例如马不走日字)，那么程序不予理会
    }
    DeleteDC(Xqwl.hdcTmp);
    ReleaseDC(Xqwl.hWnd, Xqwl.hdc);
}


// 窗体事件捕捉过程
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int x, y;
    HDC hdc;
    RECT rect;
    PAINTSTRUCT ps;
    MSGBOXPARAMS mbp;
    
    switch (uMsg) {
            // 新建窗口
        case WM_CREATE:
            // 调整窗口位置和尺寸
            GetWindowRect(hWnd, &rect);
            x = rect.left;
            y = rect.top;
            rect.right = rect.left + BOARD_WIDTH;
            rect.bottom = rect.top + BOARD_HEIGHT;
            AdjustWindowRect(&rect, WINDOW_STYLES, TRUE);
            MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
            break;
            // 退出
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            // 菜单命令
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_FILE_RED:
                case IDM_FILE_BLACK:
                    Xqwl.bFlipped = (LOWORD(wParam) == IDM_FILE_BLACK);
                    Startup();
                    hdc = GetDC(Xqwl.hWnd);
                    DrawBoard(hdc);
                    if (Xqwl.bFlipped) {
                        Xqwl.hdc = hdc;
                        Xqwl.hdcTmp = CreateCompatibleDC(Xqwl.hdc);
                        ResponseMove();
                        DeleteDC(Xqwl.hdcTmp);
                    }
                    ReleaseDC(Xqwl.hWnd, hdc);
                    break;
                case IDM_FILE_EXIT:
                    DestroyWindow(Xqwl.hWnd);
                    break;
                case IDM_HELP_HOME:
                    ShellExecute(NULL, NULL, "http://www.xqbase.com/", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case IDM_HELP_ABOUT:
                    // 弹出带象棋小巫师图标的对话框
                    MessageBeep(MB_ICONINFORMATION);
                    mbp.cbSize = sizeof(MSGBOXPARAMS);
                    mbp.hwndOwner = hWnd;
                    mbp.hInstance = Xqwl.hInst;
                    mbp.lpszText = cszAbout;
                    mbp.lpszCaption = "关于象棋小巫师";
                    mbp.dwStyle = MB_USERICON;
                    mbp.lpszIcon = MAKEINTRESOURCE(IDI_APPICON);
                    mbp.dwContextHelpId = 0;
                    mbp.lpfnMsgBoxCallback = NULL;
                    mbp.dwLanguageId = 0;
                    MessageBoxIndirect(&mbp);
                    break;
            }
            break;
            // 绘图
        case WM_PAINT:
            hdc = BeginPaint(Xqwl.hWnd, &ps);
            DrawBoard(hdc);
            EndPaint(Xqwl.hWnd, &ps);
            break;
            // 鼠标点击
        case WM_LBUTTONDOWN:
            x = FILE_LEFT + (LOWORD(lParam) - BOARD_EDGE) / SQUARE_SIZE;
            y = RANK_TOP + (HIWORD(lParam) - BOARD_EDGE) / SQUARE_SIZE;
            if (x >= FILE_LEFT && x <= FILE_RIGHT && y >= RANK_TOP && y <= RANK_BOTTOM) {
                ClickSquare(COORD_XY(x, y));
            }
            break;
            // 其他事件
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return FALSE;
}

// 装入资源图片
inline HBITMAP LoadResBmp(int nResId) {
    return (HBITMAP) LoadImage(Xqwl.hInst, MAKEINTRESOURCE(nResId), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

// 入口过程
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int i;
    MSG msg;
    WNDCLASSEX wce;
    
    // 初始化全局变量
    srand((DWORD) time(NULL));
    InitZobrist();
    Xqwl.hInst = hInstance;
    LoadBook();
    Xqwl.bFlipped = FALSE;
    Startup();
    
    // 装入图片
    Xqwl.bmpBoard = LoadResBmp(IDB_BOARD);
    Xqwl.bmpSelected = LoadResBmp(IDB_SELECTED);
    for (i = PIECE_KING; i <= PIECE_PAWN; i ++) {
        Xqwl.bmpPieces[SIDE_TAG(0) + i] = LoadResBmp(IDB_RK + i);
        Xqwl.bmpPieces[SIDE_TAG(1) + i] = LoadResBmp(IDB_BK + i);
    }
    
    // 设置窗口
    wce.cbSize = sizeof(WNDCLASSEX);
    wce.style = 0;
    wce.lpfnWndProc = (WNDPROC) WndProc;
    wce.cbClsExtra = wce.cbWndExtra = 0;
    wce.hInstance = hInstance;
    wce.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 32, 32, LR_SHARED);
    wce.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    wce.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wce.lpszMenuName = MAKEINTRESOURCE(IDM_MAINMENU);
    wce.lpszClassName = "XQWLIGHT";
    wce.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);
    RegisterClassEx(&wce);
    
    // 打开窗口
    Xqwl.hWnd = CreateWindow("XQWLIGHT", "象棋小巫师", WINDOW_STYLES,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    if (Xqwl.hWnd == NULL) {
        return 0;
    }
    ShowWindow(Xqwl.hWnd, nCmdShow);
    UpdateWindow(Xqwl.hWnd);
    
    // 接收消息
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}*/

