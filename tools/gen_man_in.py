#!/usr/bin/env python3
"""Generate man/<lang>/poedit.1.in from man/<lang>/poedit.1 with @vars@ and translated COPYRIGHT."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

L10N = {
    "de": {
        "th4": "Benutzerbefehle",
        "author": r"Geschrieben von @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
Lizenz AGPL-3.0-or-later. Dieses Programm ist freie Software: Sie dürfen es
verändern und unter der GNU Affero General Public License weiterverbreiten.
ES WIRD KEINE GARANTIE gegeben, soweit das Gesetz es zulässt. Den
vollständigen Lizenztext, einschließlich Zusatzbedingungen, die
KI-Ausbeutung und KI-Hegemonie ablehnen und triviale MIT- sowie naive
BSD-typische Lizenzen ablehnen, entnehmen Sie der Datei \fBLICENSE\fP in
der Quellveröffentlichung.""",
    },
    "eo": {
        "th4": "Ordono de uzanto",
        "author": r"Skribis @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
Permesilo AGPL-3.0 aŭ pli malfrua. Tio estas libera programaro: laŭ
kondiĉoj de la GNU Affero General Public License oni ĝin povas
modifi kaj pludistribi. SEN GARANTIO, eble permesite de leĝo. Plena
permesila teksto, inkl. kondiĉojn kontraŭ AI-ekspluatado, troviĝas
en \fBLICENSE\fP de la fontodistribuo.""",
    },
    "fr": {
        "th4": "Commandes utilisateur",
        "author": r"Écrit par @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
Licence AGPL-3.0 ou ultérieur. C'est un logiciel libre: vous pouvez
le modifier et le redistribuer selon les termes de la GNU Affero
General Public License. AUCUNE GARANTIE, dans la limite de la
loi. Le texte de licence intégral, dont des clauses
supplémentaires rejetant l'exploitation par l'IA, est dans
\fBLICENSE\fP, dans la distribution des sources.""",
    },
    "it": {
        "th4": "Comandi utente",
        "author": r"Scritto da @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
Licenza AGPL-3.0 o successiva. Questo programma è software libero:
potete modificarlo e redistribuirlo alle condizioni della GNU
Affero General Public License. NESSUNA GARANZIA, nei limiti di
legge. Il testo completo, incluse le clausole su abusi dell'IA, è
nel file \fBLICENSE\fP nella distribuzione sorgente.""",
    },
    "ja": {
        "th4": "ユーザーコマンド",
        "author": r"執筆: @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>。",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
ライセンス: AGPL-3.0 以降. 本プログラムはフリーソフトです. GNU
Affero General Public License の条件の下で変更·再配布できます. 法が
認める限り保証しません. AI 搾取等に反する追加条項を含む全文のライ
センスはソース同梱の \fBLICENSE\fP を参照.""",
    },
    "ko": {
        "th4": "사용자 명령",
        "author": r"제작: @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
AGPL-3.0-or-later. 자유 소프트웨어: GNU Affero General Public License
조건에 따라 변경·재배포할 수 있음. 법이 허용하는 범위에서
보증 없음. AI 착취 반대 등 보충 조항이 포함된 전문은 소스
배포의 \fBLICENSE\fP 파일을 읽을 것.""",
    },
    "th": {
        "th4": "คำสั่งผู้ใช้",
        "author": r"แต่งโดย @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
อนุญาตภายใต้ AGPL-3.0 หรือรุ่นใหม่. ซอฟต์แวร์เสรี: แก้ไข
และแจกต่อตาม GNU Affero General Public License. ไม่รับ
ประกันตามขอบเขตกฎหมาย. อ่านข้อความเต็มรวมข้อกำหนดเสริม ใน
แหล่งที่ \fBLICENSE\fP ของแพกเกจต้นฉบับ.""",
    },
    "vi": {
        "th4": "Lệnh người dùng",
        "author": r"Tác giả: @PROJECT_AUTHOR@ <@PROJECT_EMAIL@>.",
        "copy": r"""Copyright \(co @PROJECT_YEAR@.
Giấy phép AGPL-3.0 trở lên. Phần mềm tự do: bạn có thể sửa đổi
và phân phối lại theo GNU Affero General Public License. Không
bảo hành trong phạm vi pháp cho phép. Toàn văn, kể cả điều khoản
về bóc lột AI, xem tệp \fBLICENSE\fP trong bản phân phối
mã nguồn.""",
    },
    "zh_CN": {
        "th4": "用户命令",
        "author": r"由 @PROJECT_AUTHOR@ <@PROJECT_EMAIL@> 编写。",
        "copy": r"""版权所有 \(co @PROJECT_YEAR@.
许可: AGPL-3.0 或更新版本. 本程序为自由软件, 你可依据 GNU
Affero General Public License 修改并再发布; 在适用法律
范围内不提供保证. 完整许可(含反 AI 剥削等补充条款)见源
代发布包中的 \fBLICENSE\fP 文件。""",
    },
    "zh_TW": {
        "th4": "使用者命令",
        "author": r"由 @PROJECT_AUTHOR@ <@PROJECT_EMAIL@> 撰寫。",
        "copy": r"""版權所有 \(co @PROJECT_YEAR@.
授權: AGPL-3.0 或更新版. 本程式為自由軟體, 你可依 GNU
Affero General Public License 修改與再散布; 於法律
允許範圍內不提供保證. 完整條文(含反 AI 剝削等
補充)見源碼套件內的 \fBLICENSE\fP 。""",
    },
}


def replace_section(text: str, sh_name: str, new_body: str) -> str:
    """Replace content between .SH NAME and next .SH (exclusive start)."""
    start_key = f".SH {sh_name}\n"
    i0 = text.find(start_key)
    if i0 < 0:
        raise SystemExit(f"missing {sh_name}")
    i1 = i0 + len(start_key)
    j = text.find("\n.SH ", i1)
    if j < 0:
        rest = ""
        block = new_body.rstrip() + "\n"
    else:
        j += 1  # keep \n before .SH
        rest = text[j:]
        block = new_body.rstrip() + "\n"
    return text[:i0] + start_key + block + rest


def main() -> None:
    for lang, meta in L10N.items():
        p = ROOT / "man" / lang / "poedit.1"
        if not p.is_file():
            print("skip (no file):", p)
            continue
        t = p.read_text(encoding="utf-8")
        t4 = meta["th4"]
        t = replace_section(t, "COPYRIGHT", meta["copy"])
        t = replace_section(t, "AUTHOR", meta["author"])
        lines = t.splitlines(keepends=True)
        lines[0] = f'.TH poedit 1 "@PROJECT_YEAR@" "@PROJECT_VERSION@" "{t4}"\n'
        out = ROOT / "man" / lang / "poedit.1.in"
        out.write_text("".join(lines), encoding="utf-8")
        p.unlink()
        print("Wrote", out, "deleted", p)


if __name__ == "__main__":
    main()
