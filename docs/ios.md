# iPhone 版のビルドとインストール

PitchNet を自分の iPhone で動かすための手順です。App Store には出さず、
自分の Mac から自分の iPhone に直接インストールします。

## 必要なもの

- Mac（Apple シリコン推奨）と Xcode 16 以降（App Store から無料）
- CMake 3.28 以降（`brew install cmake`）
- iPhone（iOS 16 以降）と USB ケーブル
- Apple ID（無料で可。無料アカウントの場合、インストールしたアプリは 7 日で
  期限切れになるので、その都度 Xcode から入れ直します）

## 1. ソースを取得

```bash
git clone https://github.com/toshitoshi-creator/pitchcreate.git
cd pitchcreate
git submodule update --init third_party/JUCE third_party/r8brain-free-src
```

## 2. Xcode プロジェクトを生成

```bash
cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64
```

初回は ONNX Runtime（iOS 版）と GAME モデルを自動でダウンロードします。

Apple の Team ID が分かっていれば、`-DPITCHNET_IOS_DEVELOPMENT_TEAM=XXXXXXXXXX`
を付けると署名の設定まで済みます。分からなければ次の手順で Xcode 上で選べます。

## 3. Xcode で iPhone に入れる

1. `open build-ios/PitchNet.xcodeproj`
2. 上部のスキームで **PitchNet - App** を選び、実行先に接続した iPhone を選ぶ
3. 左のプロジェクト一覧で `PitchNet` ターゲット → **Signing & Capabilities** →
   **Team** で自分の Apple ID を選ぶ（初回は Xcode の設定で Apple ID を追加）
   - 「Bundle Identifier が使えない」と出たら `com.sessionloops.pitchnet` を
     `com.<自分の名前>.pitchnet` など一意な名前に変える
4. ▶（Run）でビルドして iPhone にインストール
5. 初回は iPhone 側で **設定 → 一般 → VPN とデバイス管理** から自分の
   開発元を「信頼」する。iOS 16 以降は **設定 → プライバシーとセキュリティ →
   デベロッパモード** もオンにする

## 使い方（iPhone 版）

画面は横向き固定です。上部のバーに主な操作があります。

| ボタン | 動作 |
| --- | --- |
| 録音 | マイクで録音開始。もう一度押すと停止し、そのまま解析して編集画面に表示 |
| 開く | ファイル App（ボイスメモから保存した音声、iCloud Drive など）から音声を選ぶ |
| 保存 | プロジェクト（.pitchnet）を保存 |
| 書き出し | 補正後の音声を書き出し、共有シートで「ファイルに保存」や AirDrop などに送る |
| メニュー | デスクトップ版のメニュー（編集・表示・設定など）すべて |

編集画面の操作:

- 1 本指: デスクトップ版のマウス操作と同じ（音符をタップで選択、上下ドラッグで
  ピッチ移動など）
- 2 本指ピンチ: 横に広げると時間方向、縦に広げると音程方向にズーム
- 2 本指ドラッグ: スクロール

ボイスメモの録音を使う場合は、ボイスメモ App で録音を選び「共有 → ファイルに
保存」してから「開く」で選ぶか、共有シートで直接 PitchNet を選びます。

録音・書き出したファイルはファイル App の「このiPhone内 → PitchNet」から
見られます（Recordings / Exports / Projects / Imported フォルダ）。

## 既知の制限

- 解析とボコーダー（再合成）は CPU で動くため、長い音声は時間がかかります。
  まずは 1 分以内のボーカルで試すのがおすすめです
- 本体は AGPLv3 です。声の合成モデル（PC-NSF-HiFiGAN）の配布条件は
  [openvpi/vocoders](https://github.com/openvpi/vocoders) で確認してください。
  個人利用の範囲で使う想定です
- DAW 用プラグイン（VST3 / AU / ARA）は iOS 版にはありません

## デスクトップでモバイル画面を試す

`-DPITCHNET_FORCE_MOBILE_UI=ON` を付けてデスクトップ版をビルドすると、
iPhone 版と同じ操作バーのレイアウトで起動します（共有の代わりに書き出し先を
Finder などで表示します）。
