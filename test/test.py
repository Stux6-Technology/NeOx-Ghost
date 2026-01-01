#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import ast
import logging
import argparse
import contextlib
import json
import os
import re
import sys
from enum import IntEnum
from pathlib import Path
from hashlib import sha256
from typing import TYPE_CHECKING, Any, Callable, ContextManager, Iterable, Iterator, Literal, Sequence, TypeVar, cast
from itertools import chain
from transformers import AutoConfig

import math
import numpy as np
import torch

if TYPE_CHECKING:
    from torch import Tensor

if 'NO_LOCAL_GGUF' not in os.environ:
    sys.path.insert(1, str(Path(__file__).parent / 'gguf-py'))
import gguf
from gguf.vocab import MistralTokenizerType, MistralVocab

try:
    from mistral_common.tokens.tokenizers.base import TokenizerVersion # pyright: ignore[reportMissingImports]
    from mistral_common.tokens.tokenizers.multimodal import DATASET_MEAN as _MISTRAL_COMMON_DATASET_MEAN, DATASET_STD as _MISTRAL_COMMON_DATASET_STD # pyright: ignore[reportMissingImports]
    from mistral_common.tokens.tokenizers.tekken import Tekkenizer # pyright: ignore[reportMissingImports]
    from mistral_common.tokens.tokenizers.sentencepiece import ( # pyright: ignore[reportMissingImports]
        SentencePieceTokenizer,
    )

    _mistral_common_installed = True
    _mistral_import_error_msg = ""
except ImportError:
    _MISTRAL_COMMON_DATASET_MEAN = (0.48145466, 0.4578275, 0.40821073)
    _MISTRAL_COMMON_DATASET_STD = (0.26862954, 0.26130258, 0.27577711)

    _mistral_common_installed = False
    TokenizerVersion = None
    Tekkenizer = None
    SentencePieceTokenizer = None
    _mistral_import_error_msg = (
        "Mistral format requires `mistral-common` to be installed. Please run "
