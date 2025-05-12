# SIC/XE 組譯器

這是一個 SIC/XE 組譯器的實作，包含兩個主要階段：Pass 1 和 Pass 2。

## 專案結構

專案分為兩個主要目錄：
- `Pass1/`: 第一階段組譯器
- `Pass2/`: 第二階段組譯器

## 功能說明

### Pass 1
第一階段組譯器主要負責：
- 讀取並解析 SIC/XE 組合語言程式碼
- 建立符號表（Symbol Table）
- 計算每個指令的位址
- 處理組合語言指示（Assembler Directives）

### Pass 2
第二階段組譯器主要負責：
- 產生目標程式碼（Object Code）
- 處理位址模式（Addressing Modes）
- 產生可重定位的目標程式
- 輸出 H、T、M、E 記錄

## 編譯與執行

### 編譯方式
```bash
# 編譯 Pass 1
cd Pass1
gcc -o main main.c

# 編譯 Pass 2
cd Pass2
gcc -o main main.c
```

### 執行方式
```bash
# 執行 Pass 1
./main <輸入檔案名稱>.asm

# 執行 Pass 2
./main <輸入檔案名稱>.asm
```

## 支援的指令格式

- Format 1 指令（1 位元組）
- Format 2 指令（2 位元組）
- Format 3 指令（3 位元組）
- Format 4 指令（4 位元組）

## 支援的定址模式

- 簡單定址（Simple Addressing）
- 立即定址（Immediate Addressing）
- 間接定址（Indirect Addressing）
- 索引定址（Indexed Addressing）

## 支援的組合語言指示

- START
- END
- BYTE
- WORD
- RESB
- RESW
- BASE
- NOBASE

## 輸出格式

### Pass 1 輸出
- 顯示每個指令的位址
- 顯示符號表
- 顯示程式長度

### Pass 2 輸出
- H 記錄：程式名稱、起始位址、程式長度
- T 記錄：目標程式碼
- M 記錄：重定位資訊
- E 記錄：程式起始位址

## 注意事項

1. 輸入檔案必須是有效的 SIC/XE 組合語言程式碼
2. 符號名稱長度限制為 20 個字元
3. 程式碼必須以 END 指示結束
4. 建議使用 START 指示指定程式起始位址 