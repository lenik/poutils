#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
PO_DIR = ROOT / "po"


TRANSLATIONS: dict[str, dict[str, str]] = {
    "de": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "Aufruf: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "Alternativen zufällig wählen\n",
        "alternate words (disable normalization)\n": "Wörter alternieren (Normalisierung deaktivieren)\n",
        "Default mode: normalize.\n": "Standardmodus: normalisieren.\n",
        "Default mode: alternate.\n": "Standardmodus: alternieren.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "Fügt Wörtern aus ARGS oder der Standardeingabe zufällige Tippfehler hinzu.\n",
        "random seed, epoch milliseconds by default\n": "Zufalls-Seed, standardmäßig Epoch-Millisekunden\n",
        "prefer QWERTY keyboard neighbors (default)\n": "QWERTY-Tastaturnachbarn bevorzugen (Standard)\n",
        "prefer Dvorak keyboard neighbors\n": "Dvorak-Tastaturnachbarn bevorzugen\n",
        "swap neighbor letters in a word, default 0\n": "Benachbarte Buchstaben in einem Wort vertauschen, Standard 0\n",
        "replace with a neighbor keyboard key, default 0\n": "Durch benachbarte Tastaturtaste ersetzen, Standard 0\n",
        "toggle case of first letter, default 0\n": "Groß-/Kleinschreibung des ersten Buchstabens umschalten, Standard 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "Wenn PERCENT >100 ist, können mehrere Tippfehler in einem Wort auftreten.\n",
    },
    "fr": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "Utilisation : %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "choisir les alternatives aléatoirement\n",
        "alternate words (disable normalization)\n": "alterner les mots (désactiver la normalisation)\n",
        "Default mode: normalize.\n": "Mode par défaut : normaliser.\n",
        "Default mode: alternate.\n": "Mode par défaut : alterner.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "Introduire des fautes de frappe aléatoires dans les mots issus de ARGS ou de l'entrée standard.\n",
        "random seed, epoch milliseconds by default\n": "graine aléatoire, millisecondes epoch par défaut\n",
        "prefer QWERTY keyboard neighbors (default)\n": "préférer les touches voisines QWERTY (par défaut)\n",
        "prefer Dvorak keyboard neighbors\n": "préférer les touches voisines Dvorak\n",
        "swap neighbor letters in a word, default 0\n": "échanger les lettres voisines dans un mot, défaut 0\n",
        "replace with a neighbor keyboard key, default 0\n": "remplacer par une touche voisine du clavier, défaut 0\n",
        "toggle case of first letter, default 0\n": "inverser la casse de la première lettre, défaut 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "Si PERCENT >100, plusieurs fautes peuvent apparaître dans un même mot.\n",
    },
    "it": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "Uso: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "scegli alternative in modo casuale\n",
        "alternate words (disable normalization)\n": "alterna parole (disattiva normalizzazione)\n",
        "Default mode: normalize.\n": "Modalità predefinita: normalizza.\n",
        "Default mode: alternate.\n": "Modalità predefinita: alterna.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "Introduce refusi casuali nelle parole da ARGS o dallo standard input.\n",
        "random seed, epoch milliseconds by default\n": "seed casuale, millisecondi epoch per impostazione predefinita\n",
        "prefer QWERTY keyboard neighbors (default)\n": "preferisci i tasti vicini QWERTY (predefinito)\n",
        "prefer Dvorak keyboard neighbors\n": "preferisci i tasti vicini Dvorak\n",
        "swap neighbor letters in a word, default 0\n": "scambia lettere vicine in una parola, predefinito 0\n",
        "replace with a neighbor keyboard key, default 0\n": "sostituisci con un tasto vicino della tastiera, predefinito 0\n",
        "toggle case of first letter, default 0\n": "inverti maiuscole/minuscole della prima lettera, predefinito 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "Se PERCENT >100, possono verificarsi più refusi nella stessa parola.\n",
    },
    "eo": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "Uzado: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "elekti alternativojn hazarde\n",
        "alternate words (disable normalization)\n": "alternigi vortojn (malŝalti normaligon)\n",
        "Default mode: normalize.\n": "Defaŭlta reĝimo: normaligi.\n",
        "Default mode: alternate.\n": "Defaŭlta reĝimo: alternigi.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "Enmetu hazardajn tajperarojn en vortojn el ARGS aŭ norma enigo.\n",
        "random seed, epoch milliseconds by default\n": "hazarda semo, epoĥaj milisekundoj defaŭlte\n",
        "prefer QWERTY keyboard neighbors (default)\n": "preferi QWERTY-klavarajn najbarojn (defaŭlte)\n",
        "prefer Dvorak keyboard neighbors\n": "preferi Dvorak-klavarajn najbarojn\n",
        "swap neighbor letters in a word, default 0\n": "interŝanĝi najbarajn literojn en vorto, defaŭlte 0\n",
        "replace with a neighbor keyboard key, default 0\n": "anstataŭigi per najbara klavara klavo, defaŭlte 0\n",
        "toggle case of first letter, default 0\n": "ŝalti usklecon de la unua litero, defaŭlte 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "Se PERCENT >100, pluraj tajperaroj povas okazi en unu vorto.\n",
    },
    "ja": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "使い方: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "候補をランダムに選択します\n",
        "alternate words (disable normalization)\n": "単語を置換モードで処理します（正規化を無効化）\n",
        "Default mode: normalize.\n": "デフォルトモード: 正規化。\n",
        "Default mode: alternate.\n": "デフォルトモード: 置換。\n",
        "Introduce random typos into words from ARGS or standard input.\n": "ARGS または標準入力の単語にランダムなタイプミスを加えます。\n",
        "random seed, epoch milliseconds by default\n": "乱数シード（デフォルトはエポックミリ秒）\n",
        "prefer QWERTY keyboard neighbors (default)\n": "QWERTY の近傍キーを優先（デフォルト）\n",
        "prefer Dvorak keyboard neighbors\n": "Dvorak の近傍キーを優先\n",
        "swap neighbor letters in a word, default 0\n": "単語内の隣接文字を入れ替え（デフォルト 0）\n",
        "replace with a neighbor keyboard key, default 0\n": "近傍キーの文字に置換（デフォルト 0）\n",
        "toggle case of first letter, default 0\n": "先頭文字の大小文字を反転（デフォルト 0）\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "PERCENT が 100 を超える場合、1 単語に複数のタイプミスが入ることがあります。\n",
    },
    "ko": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "사용법: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "대체어를 무작위로 선택합니다\n",
        "alternate words (disable normalization)\n": "단어를 대체 모드로 처리합니다(정규화 비활성화)\n",
        "Default mode: normalize.\n": "기본 모드: 정규화.\n",
        "Default mode: alternate.\n": "기본 모드: 대체.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "ARGS 또는 표준 입력의 단어에 무작위 오타를 추가합니다.\n",
        "random seed, epoch milliseconds by default\n": "랜덤 시드, 기본값은 epoch 밀리초\n",
        "prefer QWERTY keyboard neighbors (default)\n": "QWERTY 인접 키 우선(기본값)\n",
        "prefer Dvorak keyboard neighbors\n": "Dvorak 인접 키 우선\n",
        "swap neighbor letters in a word, default 0\n": "단어에서 이웃 글자 교환, 기본값 0\n",
        "replace with a neighbor keyboard key, default 0\n": "인접 키 문자로 치환, 기본값 0\n",
        "toggle case of first letter, default 0\n": "첫 글자 대소문자 전환, 기본값 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "PERCENT가 100보다 크면 한 단어에 여러 오타가 생길 수 있습니다.\n",
    },
    "th": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "วิธีใช้: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "เลือกคำทางเลือกแบบสุ่ม\n",
        "alternate words (disable normalization)\n": "สลับคำเป็นโหมดแทนที่ (ปิดการทำให้เป็นมาตรฐาน)\n",
        "Default mode: normalize.\n": "โหมดปริยาย: ทำให้เป็นมาตรฐาน\n",
        "Default mode: alternate.\n": "โหมดปริยาย: แทนที่คำ\n",
        "Introduce random typos into words from ARGS or standard input.\n": "เพิ่มคำพิมพ์ผิดแบบสุ่มให้คำจาก ARGS หรืออินพุตมาตรฐาน\n",
        "random seed, epoch milliseconds by default\n": "เมล็ดสุ่ม โดยปริยายใช้ epoch มิลลิวินาที\n",
        "prefer QWERTY keyboard neighbors (default)\n": "ใช้ปุ่มข้างเคียงแบบ QWERTY เป็นหลัก (ปริยาย)\n",
        "prefer Dvorak keyboard neighbors\n": "ใช้ปุ่มข้างเคียงแบบ Dvorak เป็นหลัก\n",
        "swap neighbor letters in a word, default 0\n": "สลับอักษรที่ติดกันในคำ ค่าเริ่มต้น 0\n",
        "replace with a neighbor keyboard key, default 0\n": "แทนที่ด้วยปุ่มคีย์บอร์ดข้างเคียง ค่าเริ่มต้น 0\n",
        "toggle case of first letter, default 0\n": "สลับตัวพิมพ์เล็ก/ใหญ่ของอักษรตัวแรก ค่าเริ่มต้น 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "ถ้า PERCENT >100 อาจเกิดคำพิมพ์ผิดมากกว่าหนึ่งครั้งในคำเดียว\n",
    },
    "vi": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "Cách dùng: %s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "chọn từ thay thế ngẫu nhiên\n",
        "alternate words (disable normalization)\n": "xử lý từ theo chế độ thay thế (tắt chuẩn hoá)\n",
        "Default mode: normalize.\n": "Chế độ mặc định: chuẩn hoá.\n",
        "Default mode: alternate.\n": "Chế độ mặc định: thay thế.\n",
        "Introduce random typos into words from ARGS or standard input.\n": "Thêm lỗi gõ ngẫu nhiên vào từ từ ARGS hoặc đầu vào chuẩn.\n",
        "random seed, epoch milliseconds by default\n": "hạt giống ngẫu nhiên, mặc định là mili giây epoch\n",
        "prefer QWERTY keyboard neighbors (default)\n": "ưu tiên phím lân cận QWERTY (mặc định)\n",
        "prefer Dvorak keyboard neighbors\n": "ưu tiên phím lân cận Dvorak\n",
        "swap neighbor letters in a word, default 0\n": "đổi chỗ chữ cái liền kề trong một từ, mặc định 0\n",
        "replace with a neighbor keyboard key, default 0\n": "thay bằng phím bàn phím lân cận, mặc định 0\n",
        "toggle case of first letter, default 0\n": "đảo hoa/thường của chữ cái đầu, mặc định 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "Nếu PERCENT >100, có thể có nhiều lỗi gõ trong một từ.\n",
    },
    "zh_CN": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "用法：%s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "随机选择替代项\n",
        "alternate words (disable normalization)\n": "替换单词（禁用正规化）\n",
        "Default mode: normalize.\n": "默认模式：正规化。\n",
        "Default mode: alternate.\n": "默认模式：替换。\n",
        "Introduce random typos into words from ARGS or standard input.\n": "为 ARGS 或标准输入中的单词引入随机错别字。\n",
        "random seed, epoch milliseconds by default\n": "随机种子，默认使用 epoch 毫秒\n",
        "prefer QWERTY keyboard neighbors (default)\n": "优先使用 QWERTY 键盘邻近键（默认）\n",
        "prefer Dvorak keyboard neighbors\n": "优先使用 Dvorak 键盘邻近键\n",
        "swap neighbor letters in a word, default 0\n": "交换单词中相邻字母，默认 0\n",
        "replace with a neighbor keyboard key, default 0\n": "替换为邻近键位字符，默认 0\n",
        "toggle case of first letter, default 0\n": "切换首字母大小写，默认 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "如果 PERCENT >100，一个单词中可能出现多个错别字。\n",
    },
    "zh_TW": {
        "Usage: %s [OPTIONS] [ARGS...]\n": "用法：%s [OPTIONS] [ARGS...]\n",
        "choose alternatives randomly\n": "隨機選擇替代項\n",
        "alternate words (disable normalization)\n": "替換單詞（停用正規化）\n",
        "Default mode: normalize.\n": "預設模式：正規化。\n",
        "Default mode: alternate.\n": "預設模式：替換。\n",
        "Introduce random typos into words from ARGS or standard input.\n": "為 ARGS 或標準輸入中的單詞加入隨機錯字。\n",
        "random seed, epoch milliseconds by default\n": "隨機種子，預設為 epoch 毫秒\n",
        "prefer QWERTY keyboard neighbors (default)\n": "優先使用 QWERTY 鍵盤鄰近鍵（預設）\n",
        "prefer Dvorak keyboard neighbors\n": "優先使用 Dvorak 鍵盤鄰近鍵\n",
        "swap neighbor letters in a word, default 0\n": "交換單詞中相鄰字母，預設 0\n",
        "replace with a neighbor keyboard key, default 0\n": "替換為鄰近鍵位字元，預設 0\n",
        "toggle case of first letter, default 0\n": "切換首字母大小寫，預設 0\n",
        "If PERCENT is >100, multiple typos may happen in one word.\n": "若 PERCENT >100，一個單詞內可能出現多個錯字。\n",
    },
}


ENTRY_PATTERN = re.compile(
    r'(?P<prefix>(?:^#.*\n)*)^msgid "(?P<msgid>(?:[^"\\]|\\.)*)"\n^msgstr "(?P<msgstr>(?:[^"\\]|\\.)*)"\n',
    re.M,
)


def update_po(path: Path, updates: dict[str, str]) -> bool:
    text = path.read_text(encoding="utf-8")
    changed = False

    def repl(m: re.Match[str]) -> str:
        nonlocal changed
        msgid = bytes(m.group("msgid"), "utf-8").decode("unicode_escape")
        if msgid not in updates:
            return m.group(0)

        new_msgstr = updates[msgid]
        escaped = (
            new_msgstr.replace("\\", "\\\\")
            .replace('"', '\\"')
            .replace("\n", "\\n")
        )

        prefix = m.group("prefix")
        if "#, fuzzy" in prefix:
            prefix = "\n".join(
                line for line in prefix.splitlines() if line.strip() != "#, fuzzy"
            )
            if prefix:
                prefix += "\n"

        out = f'{prefix}msgid "{m.group("msgid")}"\nmsgstr "{escaped}"\n'
        if out != m.group(0):
            changed = True
        return out

    new_text = ENTRY_PATTERN.sub(repl, text)
    if changed:
        path.write_text(new_text, encoding="utf-8")
    return changed


def main() -> None:
    changed_files = []
    for lang, mapping in TRANSLATIONS.items():
        po_path = PO_DIR / f"{lang}.po"
        if not po_path.exists():
            continue
        if update_po(po_path, mapping):
            changed_files.append(po_path.name)

    if changed_files:
        print("updated:", ", ".join(sorted(changed_files)))
    else:
        print("no changes")


if __name__ == "__main__":
    main()
