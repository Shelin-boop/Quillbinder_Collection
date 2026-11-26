#!/usr/bin/env python3
"""Converts the add-on's models for the engine (CTranslate2, int8) and writes their catalog.

    python3 models/convert.py <out dir> [<dictionary dir>]

Run by .github/workflows/models.yml on GitHub's runner; the files go onto the release named in RELEASE, from where
the add-on downloads the model the writer chooses. Every file is named "<model id>--<file name>" there; a file
larger than GitHub allows for one release file is cut into parts ("...--model.bin.part1", ...), which the add-on
joins after checking each. The catalog (written into <out dir> as catalog.json, copied into resources/catalog.json)
lists the models of every release with size and SHA-256 of each file; the add-on checks each download against it.
A published release is never changed: the models of "models-1" (1.0), "models-2" (1.1) and "models-3" (1.3: OPUS-MT
English to German and the German dictionary) are taken over from resources/catalog.json as they are; the new ones of this
run go onto "models-4" (1.5, the Translation Helper): the OPUS-MT models between English and Spanish, French, Portuguese,
Italian, Russian, Indonesian, Turkish, Polish and Dutch (18 models, MODELS_4 below), and, when a dictionary folder is
given (one subfolder per language, each made by tools/make-dictionary.py), the dictionary of each of those languages as a
data download. A language whose dictionary is not in the folder has none (the models' translations only).

    python3 models/convert.py <out dir> [<dictionary folder>]
    python3 models/convert.py --assemble <out dir> <dictionary folder>
                                                  adds the dictionaries to the catalog.json of a run without them (the
                                                  workflow makes the dictionaries in jobs of their own)
    python3 models/convert.py --placeholders      writes the entries of models-4 with placeholders into resources/catalog.json
                                                  (the run of the workflow writes the real sizes and sums)

Needs: ctranslate2 4.9.0, transformers 4.51 or later, torch (CPU), sentencepiece, huggingface_hub.
"""
import hashlib
import json
import os
import shutil
import subprocess
import sys

try:
    from huggingface_hub import hf_hub_download
except ImportError:   # --placeholders needs nothing of it
    hf_hub_download = None

CT2_VERSION = "4.9.0"
RELEASE = "models-4"
PART = 1_900_000_000   # GitHub takes files up to 2 GiB on a release

# The models of "models-1" (1.0), converted then; kept here as the record of how (not converted again).
MODELS_1 = [
    {
        "id": "coedit-large",
        "kind": "corrector",
        "title": "CoEdIT Large (Grammarly)",
        "summary": "Best quality: finds most spelling and grammar errors. Non-commercial use only.",
        "repo": "grammarly/coedit-large",
        "revision": "5637bcdf9d8d4419f97c8cfea36f7d35c79232b6",
        "tokenizer": {"type": "t5", "repo": "grammarly/coedit-large",
                      "revision": "5637bcdf9d8d4419f97c8cfea36f7d35c79232b6", "file": "spiece.model"},
        "license": "CC BY-NC 4.0",
        "licenseUrl": "https://creativecommons.org/licenses/by-nc/4.0/",
        "nonCommercial": True,
        "credit": "CoEdIT-large by Vipul Raheja, Dhruv Kumar, Ryan Koo and Dongyeop Kang (Grammarly), "
                  "https://huggingface.co/grammarly/coedit-large, licensed under CC BY-NC 4.0. "
                  "Changed: converted to CTranslate2 with 8-bit weights.",
        "memoryMB": 1600,
    },
    {
        "id": "coedit-base",
        "kind": "corrector",
        "title": "CoEdIT Base (free license)",
        "summary": "Smaller and faster, free for any use, but it misses many errors.",
        "repo": "jbochi/coedit-base",
        "revision": "bef93f1b9bae24f9383580764820d5de1adafcdc",
        # the model has only tokenizer.json; its vocabulary is FLAN-T5's, whose SentencePiece file is used
        # (checked below: both give the same pieces)
        "tokenizer": {"type": "t5", "repo": "google/flan-t5-base",
                      "revision": "7bcac572ce56db69c1ea7c8af255c5d7c9672fc2", "file": "spiece.model"},
        "license": "Apache-2.0",
        "licenseUrl": "https://www.apache.org/licenses/LICENSE-2.0",
        "nonCommercial": False,
        "credit": "coedit-base by J. Bochi, https://huggingface.co/jbochi/coedit-base (FLAN-T5 base trained on "
                  "Grammarly's CoEdIT data), licensed under Apache-2.0. Changed: converted to CTranslate2 with "
                  "8-bit weights.",
        "memoryMB": 600,
    },
    {
        "id": "opus-mt-de-en",
        "kind": "translator",
        "title": "OPUS-MT German to English (University of Helsinki)",
        "summary": "Translates German into English. Free for any use.",
        "repo": "Helsinki-NLP/opus-mt-de-en",
        "revision": "1a922f3b32a8e809e17a47d4b32142d8105924e5",
        "tokenizer": {"type": "marian", "repo": "Helsinki-NLP/opus-mt-de-en",
                      "revision": "1a922f3b32a8e809e17a47d4b32142d8105924e5",
                      "source": "source.spm", "target": "target.spm"},
        "license": "Apache-2.0",
        "licenseUrl": "https://www.apache.org/licenses/LICENSE-2.0",
        "nonCommercial": False,
        "credit": "opus-mt-de-en by Language Technology Research Group at the University of Helsinki "
                  "(Helsinki-NLP), https://huggingface.co/Helsinki-NLP/opus-mt-de-en, licensed under Apache-2.0. "
                  "Changed: converted to CTranslate2 with 8-bit weights.",
        "memoryMB": 300,
    },
]

# The models of "models-3" (1.3), converted then; kept as the record of how (not converted again). "revision" is pinned:
# the revision of 2026-11-23 (Hugging Face's API; the license there is CC BY 4.0).
MODELS_3 = [
    {
        "id": "opus-mt-en-de",
        "kinds": ["translator-en"],
        "type": "seq2seq",
        "title": "OPUS-MT English to German (small) (University of Helsinki)",
        "summary": "Translates English into German: the way back for Swap and Translate back, and English to German on "
                   "its own. Free for any use with credit (CC BY 4.0).",
        "repo": "Helsinki-NLP/opus-mt-en-de",
        "revision": "6183067f769a302e3861815543b9f312c71b0ca4",
        "tokenizer": {"type": "marian", "source": "source.spm", "target": "target.spm"},
        # what Hugging Face lists for that revision (2026-11-23): the SHA-256 of the large file, the git object ids of
        # the small ones; convert.py stops when a download differs
        "pins": {
            "pytorch_model.bin": {"sha256": "da068344b1b0c20e6d4a8f77f48e06646b90d679029006b18579e68977206bdd"},
            "source.spm": {"git": "e73d58f99c702eb1746e3dc55bc76255d7aa4a94"},
            "target.spm": {"git": "9dd0d88081be8d75fbc029762329cc11bfad07b9"},
            "config.json": {"git": "0a49d3f4024d5ea51560d51f29d75722d794fb16"},
            "vocab.json": {"git": "0da41bcf74fa9a6af4855c7350e27819796d9ca9"},
        },
        "license": "CC BY 4.0",
        "licenseUrl": "https://creativecommons.org/licenses/by/4.0/",
        "nonCommercial": False,
        "credit": "opus-mt-en-de by the Language Technology Research Group at the University of Helsinki "
                  "(Helsinki-NLP), https://huggingface.co/Helsinki-NLP/opus-mt-en-de, licensed under CC BY 4.0. "
                  "Changed: converted to CTranslate2 with 8-bit weights.",
    },
]


# The models of "models-4" (1.5, the Translation Helper), taken from Hugging Face on 2026-11-26 (the API's license field
# of each model, which is the license of its model card): (from, to, repository, pinned revision, license, language token,
# pins). The revision is the commit the model page shows; a pin is the SHA-256 of a large file or git's object id of a
# small one (as for MODELS_3); convert.py stops when a download differs. Where no model of a direct pair exists,
# the pair goes through English (two models; src/Languages.cpp): Portuguese to English is the model of the Romance
# languages (it knows Brazilian Portuguese: pt_br), English to Polish the model for the West Slavic languages (">>pol<<").
PAIRS_4 = [
    ('es', 'en', 'Helsinki-NLP/opus-mt-es-en', 'c96e2c5399ebfae4fc43d9669556b9afa74bb69d', 'apache-2.0', '',
     {"config.json": {"git": "03e24f5988fe1a2b9bcf9f3bbebc9bc3b4e2ac1a"}, "pytorch_model.bin": {"sha256": "586a4e69e0804459ac7c176a5dc76852652100d9af1ee0db796de719899a5deb"}, "source.spm": {"git": "2dfb31564bc31c8fbc65c8bfa8cc201fbbf45e44"}, "target.spm": {"git": "6dc92ba66940f622779f82c376066c2c46bbad12"}, "vocab.json": {"git": "d2c5a0f1ab2bb3a752894b854150c358f8c682f9"}}),
    ('en', 'es', 'Helsinki-NLP/opus-mt-en-es', '5bc4493d463cf000c1f0b50f8d56886a392ed4ab', 'apache-2.0', '',
     {"config.json": {"git": "c40dc3af1603bdbca3388970914022f4abb81f7f"}, "pytorch_model.bin": {"sha256": "befce0f620e8e63f83bbb7f0818eb51d541cba28057a7cc14b00466d85c4994c"}, "source.spm": {"git": "6dc92ba66940f622779f82c376066c2c46bbad12"}, "target.spm": {"git": "2dfb31564bc31c8fbc65c8bfa8cc201fbbf45e44"}, "vocab.json": {"git": "d2c5a0f1ab2bb3a752894b854150c358f8c682f9"}}),
    ('fr', 'en', 'Helsinki-NLP/opus-mt-fr-en', 'c4aed37b318c763fd177aa449b44e3b783cc6c02', 'apache-2.0', '',
     {"config.json": {"git": "e0e6fcaff8387b6648835ff3e326d5bc31b6e7bc"}, "model.safetensors": {"sha256": "6e3837f34b903802c3d0d670362b997cee6e87584a1108eb3fa89e4625e4424a"}, "pytorch_model.bin": {"sha256": "599b819e3488f0fb888fef09511370ce4c0388b6f0f6beeb49a1f4b19043bebc"}, "source.spm": {"git": "e95b99a0e1a115d0f27eb051c7afc5a2859f1630"}, "target.spm": {"git": "5b989d2ad3cc6596bef819b2ad755f3e06377476"}, "vocab.json": {"git": "f12d2f5f9cddfc4b05dee175269d98cab8274c12"}}),
    ('en', 'fr', 'Helsinki-NLP/opus-mt-en-fr', 'dd7f6540a7a48a7f4db59e5c0b9c42c8eea67f18', 'apache-2.0', '',
     {"config.json": {"git": "1ca77a4a9305ea071d96dd12dbe7c69c3a224f8e"}, "pytorch_model.bin": {"sha256": "cc1de10b49342ad2f33e06bc4474ddd6eaca278474903c4a8636ce15680d64de"}, "source.spm": {"git": "5b989d2ad3cc6596bef819b2ad755f3e06377476"}, "target.spm": {"git": "e95b99a0e1a115d0f27eb051c7afc5a2859f1630"}, "vocab.json": {"git": "f12d2f5f9cddfc4b05dee175269d98cab8274c12"}}),
    ('it', 'en', 'Helsinki-NLP/opus-mt-it-en', '42556a0848fc726f4d27399f20b19ff6f01afe11', 'apache-2.0', '',
     {"config.json": {"git": "0b2a697183be5b2e2847f25cb9459bc9e104a082"}, "pytorch_model.bin": {"sha256": "7b16287e467858b369c477d39498427197500a6b477e3d76c2fbcc54d915d8f3"}, "source.spm": {"git": "63f2d0839d7c24ff7fda90a27bf5abd59a1c4df6"}, "target.spm": {"git": "2bf09de715e7d0a627fb9e11ca89b1d2839f1411"}, "vocab.json": {"git": "853edcc2746b36e0a4a56947d22d013184400b0a"}}),
    ('en', 'it', 'Helsinki-NLP/opus-mt-en-it', 'be5a254d936f1a8c8da080406ca582a6615e6658', 'apache-2.0', '',
     {"config.json": {"git": "357e2a3309ea1f2f00672cbcbffb37d84b957d02"}, "pytorch_model.bin": {"sha256": "1decb91168d69a3018363d61bc7130155a2821f949e74a40b66788b5bc07d7d1"}, "source.spm": {"git": "219b146e0c09ea818d574ca669819a3b06a6077f"}, "target.spm": {"git": "760d8c46def0ffcd4979919b154d1b55cfd18a25"}, "vocab.json": {"git": "58f5353f3414392f0e0476dab6e4f9d596f9cb6e"}}),
    ('ru', 'en', 'Helsinki-NLP/opus-mt-ru-en', 'fbd6dc73284f95536648512cc21d57f19191961a', 'cc-by-4.0', '',
     {"config.json": {"git": "d898c2531e824fa65c1b4c29fa77f1bc05a5d2e0"}, "pytorch_model.bin": {"sha256": "535450eb5613f3cc912f9ca3e54cfef6c14d201b319c24a88faf776a65538b5d"}, "source.spm": {"git": "5781b8215934ee03646a5935000a19f1233210f0"}, "target.spm": {"git": "ee65931d442b9dbb755f640edb9f31b78c7c27ef"}, "vocab.json": {"git": "39833c62b978f23b3429c9109dc4ee88e339c2d5"}}),
    ('en', 'ru', 'Helsinki-NLP/opus-mt-en-ru', 'bb09c99d180016eac6819df3dae68edb1690fdee', 'apache-2.0', '',
     {"config.json": {"git": "5330276e8482c2f613b35b7c0f9caaa54f11a988"}, "pytorch_model.bin": {"sha256": "d15fa58c6bc3efd3629c1b6b86d9aa6d15d2751a4620aa4cdd7eed7b5cbe583b"}, "source.spm": {"git": "ee65931d442b9dbb755f640edb9f31b78c7c27ef"}, "target.spm": {"git": "5781b8215934ee03646a5935000a19f1233210f0"}, "vocab.json": {"git": "39833c62b978f23b3429c9109dc4ee88e339c2d5"}}),
    ('id', 'en', 'Helsinki-NLP/opus-mt-id-en', '9a7f1b0d0dfe0a92ba691030b01d6f23f966e7ec', 'apache-2.0', '',
     {"config.json": {"git": "e309324fe3e95df4ba26ae83b5798e9777532cc1"}, "pytorch_model.bin": {"sha256": "798ff872a045346ff907fc9938ecd8d8dc9b346358cc514b35e08387776bee24"}, "source.spm": {"git": "6c0ae9890c02ee98ad8a27d114cd3c38e8745c98"}, "target.spm": {"git": "eface9e96b0165a12b8e484ea7398d2d6c799747"}, "vocab.json": {"git": "e91f0583eeaffe0dfe04fc7bd0b0c7878c171bd8"}}),
    ('en', 'id', 'Helsinki-NLP/opus-mt-en-id', '6e4c52d61a6b16fe3509b0267cbfec65011b860b', 'apache-2.0', '',
     {"config.json": {"git": "6944505b7811ddd00a407d98119974460374e527"}, "pytorch_model.bin": {"sha256": "55cf1d883321cfd3bcfc43ce37fa842fc01690c7cb05afaff0fc9ca2df15f859"}, "source.spm": {"git": "eface9e96b0165a12b8e484ea7398d2d6c799747"}, "target.spm": {"git": "6c0ae9890c02ee98ad8a27d114cd3c38e8745c98"}, "vocab.json": {"git": "e91f0583eeaffe0dfe04fc7bd0b0c7878c171bd8"}}),
    ('nl', 'en', 'Helsinki-NLP/opus-mt-nl-en', '48af999f2c59b10c05ca6e008dcedc07677a9b15', 'apache-2.0', '',
     {"config.json": {"git": "9f47dbda5593157678ee8e7b8a73f6fada3d3b6a"}, "pytorch_model.bin": {"sha256": "70243188530be305eaee44a120fc768a1f7f52abad2f7bb4f87b70bd4fe2a6c1"}, "source.spm": {"git": "64f360449c391e87286744ec710e2e07b9ac890c"}, "target.spm": {"git": "35d1d085b445c5450c96ca6cbccbfabc5d52a54d"}, "vocab.json": {"git": "efb9ebbe8b3a09e6898da519fa7b8d064d99872c"}}),
    ('en', 'nl', 'Helsinki-NLP/opus-mt-en-nl', '8aad73b34ff36c090e7fc8a2eb7e2e7cca235d31', 'apache-2.0', '',
     {"config.json": {"git": "c9303d2869222502358a4e004168c1a3770d97d8"}, "pytorch_model.bin": {"sha256": "8b2ff97027f9b35904984dca8508ab633dfffc4e58c7fbedb7eb236d2a937a36"}, "source.spm": {"git": "35d1d085b445c5450c96ca6cbccbfabc5d52a54d"}, "target.spm": {"git": "64f360449c391e87286744ec710e2e07b9ac890c"}, "vocab.json": {"git": "efb9ebbe8b3a09e6898da519fa7b8d064d99872c"}}),
    ('pl', 'en', 'Helsinki-NLP/opus-mt-pl-en', '7f2bb874fdfb6139f9842b91a9b75c4a6c93401c', 'apache-2.0', '',
     {"config.json": {"git": "f28b19c3d95ae75dcf229219203c078e779951e6"}, "pytorch_model.bin": {"sha256": "37b363be48e994dce37575cfe952e1c8394ba2efc8e08933264bcfa2caf0ae9c"}, "source.spm": {"git": "bb8edbcc885a4194715985a2bb6704d1b977fb39"}, "target.spm": {"git": "6c7dd711d6df59cf02dd653b064809665921891b"}, "vocab.json": {"git": "387425b0c04a6452c8f6fce84de076bf777dc5a3"}}),
    ('en', 'pl', 'Helsinki-NLP/opus-mt-en-zlw', '92ffd5aa936924688bd715b26d6bb75393082c03', 'apache-2.0', '>>pol<<',
     {"config.json": {"git": "d625a2703d35d02f638ebe60a191c091f152c021"}, "pytorch_model.bin": {"sha256": "3352b4effbf57c85b620ac2b1c41a1e5232c0bfcb399191fcb91e081a5ca5a31"}, "source.spm": {"git": "70c5075cba456fadee4d8db3bb7b825665a481e8"}, "target.spm": {"git": "0ae442f7efa5a5eaa7692e332984fb36fc853194"}, "vocab.json": {"git": "107b3c0653cf1ccfe04d64ca9d2f796140b5a6f3"}}),
    ('tr', 'en', 'Helsinki-NLP/opus-mt-tr-en', '19c65427cc2af5f191337d4899e0348c4af25902', 'apache-2.0', '',
     {"config.json": {"git": "c2f6722d72a2698df7929cbdf7a3a214713e5a8c"}, "pytorch_model.bin": {"sha256": "e2b820f31c9348b0ae33d77ac23bd1686fa60a8afa0b9511bc7d9a35c125b534"}, "source.spm": {"git": "43d4d14a2111b785c57e7c8c8233ebae6a9468c8"}, "target.spm": {"git": "958becd6921f86a3c31226bca55d28ef9c5a2f4f"}, "vocab.json": {"git": "05b435f7292581db5ce61a2aa14ae27f19f34c1d"}}),
    ('en', 'tr', 'Helsinki-NLP/opus-mt-tc-big-en-tr', 'e539fc16a8a1a0ea5950eb339b595bfcce990e90', 'cc-by-4.0', '',
     {"config.json": {"git": "e2229e55db12a9a5284018e443f1f4afed4efe43"}, "pytorch_model.bin": {"sha256": "7bc7e1973431752bcbb083c090c3c8f1e6eb3389d2e2e0faf9bf1a85d969a789"}, "source.spm": {"sha256": "180d3f94ddfee5585d78b702290ded0f4f9bd40f299349c8383b32a579fefad4"}, "target.spm": {"sha256": "cd0c475692178b6c626aadaef01bd8f98d8053946938e4e67005fc1e4811d797"}, "vocab.json": {"git": "0dc21962e176abd434d8e6eeb05a971259bbf8b2"}}),
    ('pt', 'en', 'Helsinki-NLP/opus-mt-ROMANCE-en', 'e9ca9975e3972afd80732f08ce01d3a1339f47f8', 'apache-2.0', '',
     {"config.json": {"git": "d53a30938309af55bb3dc44e59034a3acfd620ba"}, "pytorch_model.bin": {"sha256": "9d77bbbd43a214959e027ffc8713fbe31f8609d14827fba645f1361ca20a6f3a"}, "source.spm": {"git": "9080d95ae1341be221ff48919ea4a362970ac9a0"}, "target.spm": {"git": "090397efd72233e61e84abe704ff4d3146f451db"}, "vocab.json": {"git": "da7a7e613f68cd0373521753d65267e3ae6c345f"}}),
    ('en', 'pt', 'Helsinki-NLP/opus-mt-tc-big-en-pt', '9f2863d807ecf91a374bdbecb8d01e402e90622e', 'cc-by-4.0', '>>pob<<',
     {"config.json": {"git": "59a7056d6a45fc99f6e5a67ac17a1d5d8f85e530"}, "model.safetensors": {"sha256": "f1772ec97f6cb5b942bb6a5555a04272960a228a523f7ed47e24014236aa1716"}, "pytorch_model.bin": {"sha256": "076fefd494f4fb88872edbed7fc3c8203e556a7e7342aabc5ede9a73425e0eca"}, "source.spm": {"sha256": "7a7fcf812cf03a5785daa35d4932bbbe69e7e605c0fe56fce5a3f731d6c355aa"}, "target.spm": {"sha256": "b3cf20ccd4e48a89882cfdba157c05fa1cd71a6b69517dfc80e3a34d387ea91f"}, "vocab.json": {"git": "5cd4d0600f34c66f46a449ed68ae89b54b872d1c"}}),
]

LANGUAGE_NAMES = {"en": "English", "de": "German", "es": "Spanish", "fr": "French", "pt": "Portuguese (Brazil)",
                  "it": "Italian", "ru": "Russian", "id": "Indonesian", "tr": "Turkish", "pl": "Polish", "nl": "Dutch"}
# a sentence of each language for the preview in the log
SAMPLE_TEXT = {
    "en": "The rain had not stopped for three days.", "es": "La lluvia no hab\u00eda parado en tres d\u00edas.",
    "fr": "La pluie n'avait pas cess\u00e9 depuis trois jours.", "pt": "A chuva n\u00e3o parava havia tr\u00eas dias.",
    "it": "La pioggia non si fermava da tre giorni.",
    "ru": "\u0414\u043e\u0436\u0434\u044c \u043d\u0435 \u043f\u0440\u0435\u043a\u0440\u0430\u0449\u0430\u043b\u0441\u044f \u0442\u0440\u0438 \u0434\u043d\u044f.",
    "id": "Hujan belum berhenti selama tiga hari.", "tr": "Ya\u011fmur \u00fc\u00e7 g\u00fcnd\u00fcr durmam\u0131\u015ft\u0131.",
    "pl": "Deszcz nie przestawa\u0142 padać od trzech dni.", "nl": "De regen was al drie dagen niet gestopt.",
}
LICENSE_URLS = {"Apache-2.0": "https://www.apache.org/licenses/LICENSE-2.0",
                "CC BY 4.0": "https://creativecommons.org/licenses/by/4.0/"}
LICENSE_NAMES = {"apache-2.0": "Apache-2.0", "cc-by-4.0": "CC BY 4.0"}   # the API's names, as the catalog writes them
DICTIONARY_LANGUAGES = ["es", "fr", "pt", "it", "ru", "id", "tr", "pl", "nl"]
# an estimate for the catalog's placeholders (the run measures the real numbers)
ESTIMATE_MB = {"small": (79, 251), "big": (120, 340)}


def models_4():
    """The catalog entries (without files) of the models of models-4."""
    out = []
    for frm, to, repo, rev, lic, token, pins in PAIRS_4:
        lic = LICENSE_NAMES[lic]
        name = repo.split("/")[1]
        pair = f"{LANGUAGE_NAMES[frm]} to {LANGUAGE_NAMES[to]}"
        note = ""
        if "ROMANCE" in name:
            note = " It is the model for the Romance languages; here it is used for Portuguese."
        elif name.endswith("zlw"):
            note = " It is the model for the West Slavic languages; here it is told to write Polish (>>pol<<)."
        elif "tc-big" in name:
            note = " The larger OPUS-MT model for this pair."
        out.append({
            "id": f"opus-mt-{frm}-{to}",
            "kinds": ["translator-pair"],
            "type": "seq2seq",
            "from": frm,
            "to": to,
            "title": f"OPUS-MT {pair} (University of Helsinki)",
            "summary": f"Translates {LANGUAGE_NAMES[frm]} into {LANGUAGE_NAMES[to]}; a pair without English goes through "
                       f"English (two models). Free for any use with credit ({lic}).{note}",
            "repo": repo, "revision": rev, "pins": pins,
            "tokenizer": {"type": "marian", "source": "source.spm", "target": "target.spm"},
            "sourceToken": token,
            "license": lic, "licenseUrl": LICENSE_URLS[lic], "nonCommercial": False,
            "credit": f"{name} by the Language Technology Research Group at the University of Helsinki (Helsinki-NLP), "
                      f"https://huggingface.co/{repo}, licensed under {lic}. Changed: converted to CTranslate2 with 8-bit weights.",
            "big": "tc-big" in name,
        })
    return out


# The files a model of this run is made of (the converter writes the vocabulary of a shared one as shared_vocabulary.json)
MODEL_FILES = ["MODEL-LICENSE.txt", "config.json", "model.bin", "shared_vocabulary.json", "source.spm", "target.spm"]
DICTIONARY_FILES = ["DICTIONARY-LICENSE.txt", "LICENSE-CC-BY-SA-4.0.txt"]   # and <code>-dictionary.qz


def placeholder_catalog(catalog_path):
    """Writes the entries of models-4 into resources/catalog.json with placeholders for what the run measures: the sizes
    (0) and SHA-256 sums ("PLACEHOLDER-<asset>"). The add-on says "not published yet" for such a model and downloads
    nothing; the workflow's step "Write the catalog" (this script, the same run) makes the real entries, and the sizes
    and sums of its artifact go into this file after the run."""
    with open(catalog_path, encoding="utf-8") as f:
        cat = json.load(f)
    cat["release"] = RELEASE
    cat["models"] = [e for e in cat["models"] if e.get("release") != RELEASE]
    def files(prefix, names):
        return [{"name": n, "asset": f"{prefix}--{n}", "size": 0, "sha256": f"PLACEHOLDER-{prefix}--{n}"} for n in names]
    for m in models_4():
        mb = ESTIMATE_MB["big" if m.pop("big") else "small"]
        for k in ("repo", "revision", "pins"):
            m.pop(k)
        if not m["sourceToken"]:
            m.pop("sourceToken")
        m["release"] = RELEASE
        m["memoryMB"] = mb[1]
        m["source"] = "https://huggingface.co/" + [p for p in PAIRS_4 if f"opus-mt-{p[0]}-{p[1]}" == m["id"]][0][2] + "/tree/" + \
            [p for p in PAIRS_4 if f"opus-mt-{p[0]}-{p[1]}" == m["id"]][0][3]
        m["files"] = files(m["id"], MODEL_FILES)
        m["sizeMB"] = mb[0]
        cat["models"].append(m)
    for code in DICTIONARY_LANGUAGES:
        cat["models"].append(dictionary_entry(code, files(f"wiktionary-{code}", DICTIONARY_FILES + [f"{code}-dictionary.qz"]), 0))
    with open(catalog_path, "w", encoding="utf-8") as f:
        json.dump(cat, f, indent=2, ensure_ascii=False)
        f.write("\n")


def dictionary_entry(code, files, size_mb):
    name = LANGUAGE_NAMES[code]
    return {
        "id": f"wiktionary-{code}", "kinds": [f"dictionary-{code}"], "type": "data", "language": code,
        "title": f"{name} dictionary (extract of the Wiktionary)",
        "summary": f"Word classes, meanings and English words for {name} words; used first when you translate a single "
                   f"{name} word. Licensed under CC BY-SA 4.0: the license is shown before the download.",
        "license": "CC BY-SA 4.0", "licenseUrl": "https://creativecommons.org/licenses/by-sa/4.0/",
        "nonCommercial": False, "askLicense": True,
        "credit": f"Dictionary data from the English Wiktionary (en.wiktionary.org) by its contributors, taken from the "
                  f"extract of kaikki.org (made with wiktextract), licensed under CC BY-SA 4.0. Changed: reduced to word "
                  f"classes, English words and definitions, and inflected forms ({name} entries).",
        "release": RELEASE, "memoryMB": 0, "source": f"https://kaikki.org/dictionary/{name.split(' ')[0]}/",
        "files": files, "sizeMB": size_mb,
    }

# The models of "models-2" (1.1), converted then; kept as the record of how (not converted again).
# "revision": None took the newest revision at conversion time and recorded it.
MODELS_2 = [
    {
        "id": "qwen3-1.7b",
        "kinds": ["corrector", "translator"],
        "type": "chat",
        "title": "Qwen3 1.7B (Qwen team, Alibaba Cloud)",
        "summary": "A small general model told to fix only real errors and to leave your style and dialogue alone; "
                   "it also translates German. Free for any use.",
        "repo": "Qwen/Qwen3-1.7B",
        "revision": None,
        "chat": {"template": "chatml", "thinkTags": True},
        "license": "Apache-2.0",
        "licenseUrl": "https://www.apache.org/licenses/LICENSE-2.0",
        "nonCommercial": False,
        "credit": "Qwen3-1.7B by the Qwen team (Alibaba Cloud), https://huggingface.co/Qwen/Qwen3-1.7B, licensed under "
                  "Apache-2.0. Changed: converted to CTranslate2 with 8-bit weights.",
    },
    {
        "id": "qwen3-4b",
        "kinds": ["corrector", "translator"],
        "type": "chat",
        "title": "Qwen3 4B (Qwen team, Alibaba Cloud)",
        "summary": "A larger general model told to fix only real errors and to leave your style and dialogue alone; it "
                   "also translates German. Better but slower; needs a computer with 8 GB of memory or more. Free "
                   "for any use.",
        "repo": "Qwen/Qwen3-4B-Instruct-2507",
        "revision": None,
        "chat": {"template": "chatml", "thinkTags": False},
        "license": "Apache-2.0",
        "licenseUrl": "https://www.apache.org/licenses/LICENSE-2.0",
        "nonCommercial": False,
        "credit": "Qwen3-4B-Instruct-2507 by the Qwen team (Alibaba Cloud), "
                  "https://huggingface.co/Qwen/Qwen3-4B-Instruct-2507, licensed under Apache-2.0. Changed: converted "
                  "to CTranslate2 with 8-bit weights.",
    },
    {
        "id": "opus-mt-big-de-en",
        "kinds": ["translator"],
        "type": "seq2seq",
        "title": "OPUS-MT Big, German to English (University of Helsinki)",
        # 1.3: the description names what the model is not good at; "forWords": False keeps single words and short
        # texts away from it (they go to the small model; the catalog carries the flag)
        "summary": "Better for whole sentences and passages (CC BY 4.0, free for any use with credit); unreliable for "
                   "single words and short phrases, which the add-on always gives to the small model.",
        "forWords": False,
        "repo": "Helsinki-NLP/opus-mt-tc-big-gmw-gmw",
        "revision": None,
        "tokenizer": {"type": "marian", "source": "source.spm", "target": "target.spm"},
        "sourceToken": ">>eng<<",
        "license": "CC BY 4.0",
        "licenseUrl": "https://creativecommons.org/licenses/by/4.0/",
        "nonCommercial": False,
        "credit": "opus-mt-tc-big-gmw-gmw by the Language Technology Research Group at the University of Helsinki "
                  "(Helsinki-NLP), https://huggingface.co/Helsinki-NLP/opus-mt-tc-big-gmw-gmw, licensed under CC BY "
                  "4.0. Changed: converted to CTranslate2 with 8-bit weights; used for German to English only.",
    },
]

SAMPLES = [
    "The rain had not stopped for three days, and Mara was starting to loose her patience.",
    "“I dont think he is coming,” she said quietly — more to herself then to anyone else.",
    "Der Regen hatte seit drei Tagen nicht aufgehört; „Ich glaube nicht, dass er kommt“, sagte sie.",
    "“You ever been to the sea?” she asked. “Once. It were cold.”",
    "\n\n  Two spaces, a tab\tand <tags> & numbers 12345 – done.",
]

# What a chat model is told (the add-on's own instructions are in src/Panel.cpp; these are for the preview only).
PREVIEW_SYSTEM = ("You are a careful proofreader of English fiction. Correct only clear errors of spelling, grammar "
                  "and punctuation. Keep the author's words, style and dialogue. Answer with the corrected text only.")
PREVIEW = [
    "She turned around and looked at him. There was not much to loose.",
    "“Well that's easy. First, you need to find a river. Then if would be helpful if it could even talk.”",
    "He don't know where the keys is.",
]


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def check_t5_tokenizer(m, spm_file):
    """The SentencePiece file must split text exactly as the model's own tokenizer does."""
    import sentencepiece as spm
    from transformers import AutoTokenizer
    sp = spm.SentencePieceProcessor(model_file=spm_file)
    tok = AutoTokenizer.from_pretrained(m["repo"], revision=m["revision"])
    for s in SAMPLES:
        a = sp.encode(s, out_type=str)
        b = tok.convert_ids_to_tokens(tok.encode(s, add_special_tokens=False))
        if a != b:
            sys.exit(f"{m['id']}: tokenizer differs for {s!r}:\n  {a}\n  {b}")


def verify_pins(m, rev):
    """The downloaded originals must be the files the pins name (SHA-256, or git's object id for small files)."""
    for name, pin in m.get("pins", {}).items():
        path = hf_hub_download(m["repo"], name, revision=rev)
        if "sha256" in pin:
            ok = sha256(path) == pin["sha256"]
        else:
            data = open(path, "rb").read()
            ok = hashlib.sha1(b"blob %d\0" % len(data) + data).hexdigest() == pin["git"]
        print(f"  pin {name}: {'ok' if ok else 'DIFFERENT'}", flush=True)
        if not ok:
            sys.exit(f"{m['id']}: {name} is not the file the pin names (revision {rev})")


def resolve(m):
    from huggingface_hub import model_info
    if not m.get("revision"):
        m["revision"] = model_info(m["repo"]).sha
    return m["revision"]


def clear_hf_cache():
    """The runner's disk is small: the originals go once a model is converted."""
    cache = os.path.join(os.path.expanduser("~"), ".cache", "huggingface")
    shutil.rmtree(cache, ignore_errors=True)


def tokenizer_check(m, d):
    """What the model's own tokenizer makes of a few texts and of a whole chat prompt: the add-on's tokenizer must
    give the same pieces (tests/logic_tests.cpp reads this file)."""
    from transformers import AutoTokenizer
    tok = AutoTokenizer.from_pretrained(m["repo"], revision=m["revision"])
    texts = list(SAMPLES) + PREVIEW
    msgs = [{"role": "system", "content": PREVIEW_SYSTEM}, {"role": "user", "content": PREVIEW[1]}]
    kwargs = {"enable_thinking": False} if m["chat"]["thinkTags"] else {}
    prompt = tok.apply_chat_template(msgs, tokenize=False, add_generation_prompt=True, **kwargs)
    texts.append(prompt)
    out = {"model": m["id"], "samples": []}
    for t in texts:
        out["samples"].append({"text": t, "tokens": tok.convert_ids_to_tokens(tok.encode(t, add_special_tokens=False))})
    with open(os.path.join(d, "tokenizer-check.json"), "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=1)
    return prompt


def preview(m, d):
    """A few results of the converted model, for the log (the review reads them)."""
    import ctranslate2
    if m["type"] == "chat":
        from transformers import AutoTokenizer
        tok = AutoTokenizer.from_pretrained(m["repo"], revision=m["revision"])
        gen = ctranslate2.Generator(d, device="cpu", compute_type="int8")
        kwargs = {"enable_thinking": False} if m["chat"]["thinkTags"] else {}
        for text in PREVIEW:
            msgs = [{"role": "system", "content": PREVIEW_SYSTEM}, {"role": "user", "content": text}]
            prompt = tok.apply_chat_template(msgs, tokenize=False, add_generation_prompt=True, **kwargs)
            tokens = tok.convert_ids_to_tokens(tok.encode(prompt, add_special_tokens=False))
            r = gen.generate_batch([tokens], max_length=128, include_prompt_in_result=False,
                                   end_token=["<|im_end|>", "<|endoftext|>"])[0]
            print(f"  {text!r}\n    -> {tok.convert_tokens_to_string(r.sequences[0])!r}")
    else:
        import sentencepiece as spm
        src = spm.SentencePieceProcessor(model_file=os.path.join(d, m["tokenizer"]["source"]))
        tgt = spm.SentencePieceProcessor(model_file=os.path.join(d, m["tokenizer"]["target"]))
        tr = ctranslate2.Translator(d, device="cpu", compute_type="int8")
        texts = ["Der Regen hatte seit drei Tagen nicht aufgehört.", "„Ich glaube nicht, dass er kommt“, sagte sie."]
        if m["kinds"] == ["translator-en"]:
            texts = ["The rain had not stopped for three days.", "\"I don't think he is coming,\" she said.", "curse"]
        if m.get("from"):
            texts = [SAMPLE_TEXT[m["from"]]]
        for text in texts:
            pieces = ([m["sourceToken"]] if m.get("sourceToken") else []) + src.encode(text, out_type=str) + ["</s>"]
            r = tr.translate_batch([pieces], beam_size=4)[0]
            print(f"  {text!r}\n    -> {tgt.decode(r.hypotheses[0])!r}")


def memory_mb(m, model_bin):
    mb = os.path.getsize(model_bin) / 1e6
    # int8 weights in memory, plus the work memory; chat models keep a cache of the text so far
    return int(mb * 1.2 + 700) if m["type"] == "chat" else int(mb * 2 + 100)


def add_dictionaries(catalog, out, folder):
    """The dictionaries of the languages that have one: a data download each, beside the models. The folder has a
    subfolder per language (tools/make-dictionary.py writes it): <code>-dictionary.qz, DICTIONARY-LICENSE.txt and
    LICENSE-CC-BY-SA-4.0.txt. A language without a subfolder has no dictionary (its models' translations only)."""
    for code in DICTIONARY_LANGUAGES:
        d = os.path.join(folder, code)
        if not os.path.isdir(d):
            print(f"wiktionary-{code}: no dictionary was made for this language", flush=True)
            continue
        files = []
        for name in ("DICTIONARY-LICENSE.txt", "LICENSE-CC-BY-SA-4.0.txt", f"{code}-dictionary.qz"):
            src = os.path.join(d, name)
            asset = f"wiktionary-{code}--{name}"
            shutil.copy(src, os.path.join(out, asset))
            files.append({"name": name, "asset": asset, "size": os.path.getsize(src), "sha256": sha256(src)})
        entry = dictionary_entry(code, files, round(sum(f["size"] for f in files) / 1e6))
        catalog["models"].append(entry)
        print(f"wiktionary-{code}: {entry['sizeMB']} MB", flush=True)


def main():
    here0 = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if sys.argv[1:] == ["--placeholders"]:
        placeholder_catalog(os.path.join(here0, "resources", "catalog.json"))
        return
    if sys.argv[1:2] == ["--assemble"]:
        out, folder = sys.argv[2], sys.argv[3]
        path = os.path.join(out, "catalog.json")
        with open(path, encoding="utf-8") as f:
            catalog = json.load(f)
        catalog["models"] = [e for e in catalog["models"] if e["id"] not in {f"wiktionary-{c}" for c in DICTIONARY_LANGUAGES}]
        add_dictionaries(catalog, out, folder)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(catalog, f, indent=2, ensure_ascii=False)
            f.write("\n")
        return
    out = sys.argv[1]
    dictionary = sys.argv[2] if len(sys.argv) > 2 else None
    os.makedirs(out, exist_ok=True)
    work = os.path.join(out, ".work")
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    with open(os.path.join(here, "resources", "catalog.json"), encoding="utf-8") as f:
        old = json.load(f)
    # "release": the release of this run (the workflow uploads to it); each model names its own release
    catalog = {"format": 2, "release": RELEASE, "engine": f"CTranslate2 {CT2_VERSION}", "models": []}
    for e in old["models"]:
        if e.get("release", old.get("release", "models-1")) == RELEASE:
            continue   # made again below
        e.setdefault("release", old.get("release", "models-1"))
        if "kinds" not in e:
            e["kinds"] = [e.pop("kind")]
        e.setdefault("type", "seq2seq")
        catalog["models"].append(e)
    for m in models_4():
        m.pop("big", None)
        if not m["sourceToken"]:
            m.pop("sourceToken")
        d = os.path.join(work, m["id"])
        shutil.rmtree(d, ignore_errors=True)
        rev = resolve(m)
        verify_pins(m, rev)
        cmd = ["ct2-transformers-converter", "--model", m["repo"], "--revision", rev, "--output_dir", d,
               "--quantization", "int8", "--force", "--low_cpu_mem_usage"]
        if m["type"] == "chat":
            cmd += ["--copy_files", "tokenizer.json"]
        subprocess.run(cmd, check=True)
        tok_entry = {}
        if m["type"] == "chat":
            tokenizer_check(m, d)
            tok_entry = {"type": "bpe", "file": "tokenizer.json"}
        else:
            t = m["tokenizer"]
            for n in ("source", "target"):
                shutil.copy(hf_hub_download(m["repo"], t[n], revision=rev), os.path.join(d, t[n]))
            tok_entry = dict(t)
            vocab = json.load(open(os.path.join(d, "shared_vocabulary.json"), encoding="utf-8")) \
                if os.path.exists(os.path.join(d, "shared_vocabulary.json")) else \
                json.load(open(os.path.join(d, "source_vocabulary.json"), encoding="utf-8"))
            if m.get("sourceToken") and m["sourceToken"] not in vocab:
                sys.exit(f"{m['id']}: {m['sourceToken']} is not in the vocabulary")
        print(f"{m['id']} ({m['repo']} at {rev}):", flush=True)
        preview(m, d)
        with open(os.path.join(d, "MODEL-LICENSE.txt"), "w", encoding="utf-8") as f:
            f.write(f"{m['title']}\n\n{m['credit']}\n\nLicense: {m['license']} ({m['licenseUrl']})\n"
                    f"Original: https://huggingface.co/{m['repo']} at revision {rev}\n")
        files = []
        for name in sorted(os.listdir(d)):
            p = os.path.join(d, name)
            entry = {"name": name, "asset": f"{m['id']}--{name}", "size": os.path.getsize(p), "sha256": sha256(p)}
            if entry["size"] > PART:
                entry["asset"] = ""
                entry["parts"] = []
                with open(p, "rb") as src:
                    i = 1
                    while True:
                        block = src.read(PART)
                        if not block:
                            break
                        asset = f"{m['id']}--{name}.part{i}"
                        with open(os.path.join(out, asset), "wb") as dst:
                            dst.write(block)
                        entry["parts"].append({"asset": asset, "size": len(block),
                                               "sha256": hashlib.sha256(block).hexdigest()})
                        i += 1
                os.remove(p)
            else:
                shutil.move(p, os.path.join(out, entry["asset"]))
            files.append(entry)
        e = {k: m[k] for k in ("id", "kinds", "type", "title", "summary", "license", "licenseUrl", "nonCommercial",
                               "credit")}
        for k in ("from", "to"):
            if m.get(k):
                e[k] = m[k]
        e["release"] = RELEASE
        e["memoryMB"] = memory_mb(m, os.path.join(out, f"{m['id']}--model.bin")) \
            if os.path.exists(os.path.join(out, f"{m['id']}--model.bin")) else \
            int(sum(f["size"] for f in files if f["name"] == "model.bin") / 1e6 * (1.2 if m["type"] == "chat" else 2)
                + (700 if m["type"] == "chat" else 100))
        e["source"] = f"https://huggingface.co/{m['repo']}/tree/{rev}"
        e["tokenizer"] = tok_entry
        if m["type"] == "chat":
            e["chat"] = m["chat"]
        if m.get("sourceToken"):
            e["sourceToken"] = m["sourceToken"]
        e["files"] = files
        e["sizeMB"] = round(sum(f["size"] for f in files) / 1e6)
        catalog["models"].append(e)
        print(f"{m['id']}: {e['sizeMB']} MB, {len(files)} files, about {e['memoryMB']} MB of memory", flush=True)
        shutil.rmtree(d, ignore_errors=True)
        clear_hf_cache()
    shutil.rmtree(work, ignore_errors=True)
    if dictionary:
        add_dictionaries(catalog, out, dictionary)
    with open(os.path.join(out, "catalog.json"), "w", encoding="utf-8") as f:
        json.dump(catalog, f, indent=2, ensure_ascii=False)
        f.write("\n")


if __name__ == "__main__":
    main()
