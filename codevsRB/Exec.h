#pragma once

struct ExecOptions {
    bool shuffleFirstCommand;
};
extern ExecOptions execOptions;

// fin，foutを通じて実行する
// fin  : stdin
// fout : stdout
void execAI(istream& fin, ostream& fout);

// シミュレータの動作確認
// fin : 自分への入力データを指すfd
// fme : 自分の出力結果ファイルのfd
// fen : 相手の出力結果ファイルのfd
void validateSimulator(istream& fin, istream& fme, istream& fen);

// 入力を自動生成して実行
// 敵AIなし，敵フィールドは常にリセット
// 実行時間確認など
void selfExecution();
