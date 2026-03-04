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
        "`pip install mistral-common[image,audio]` to install it."
    )


logger = logging.getLogger("hf-to-gguf")


###### MODEL DEFINITIONS ######

class SentencePieceTokenTypes(IntEnum):
    NORMAL = 1
    UNKNOWN = 2
    CONTROL = 3
    USER_DEFINED = 4
    UNUSED = 5
    BYTE = 6


class ModelType(IntEnum):
    TEXT = 1
    MMPROJ = 2


AnyModel = TypeVar("AnyModel", bound="type[ModelBase]")


class ModelBase:
    _model_classes: dict[ModelType, dict[str, type[ModelBase]]] = {
        ModelType.TEXT: {},
        ModelType.MMPROJ: {},
    }

    dir_model: Path
    ftype: gguf.LlamaFileType
    fname_out: Path
    is_big_endian: bool
    endianess: gguf.GGUFEndian
    use_temp_file: bool
    lazy: bool
    dry_run: bool
    hparams: dict[str, Any]
    model_tensors: dict[str, Callable[[], Tensor]]
    gguf_writer: gguf.GGUFWriter
    model_name: str | None
    metadata_override: Path | None
    dir_model_card: Path
    remote_hf_model_id: str | None

    # subclasses should define this!
    model_arch: gguf.MODEL_ARCH

    # subclasses should initialize this!
    block_count: int
    tensor_map: gguf.TensorNameMap

    # Mistral format specifics
    is_mistral_format: bool = False
    disable_mistral_community_chat_template: bool = False
    sentence_transformers_dense_modules: bool = False

    def __init__(self, dir_model: Path, ftype: gguf.LlamaFileType, fname_out: Path, *, is_big_endian: bool = False,
                 use_temp_file: bool = False, eager: bool = False,
                 metadata_override: Path | None = None, model_name: str | None = None,
                 split_max_tensors: int = 0, split_max_size: int = 0, dry_run: bool = False,
                 small_first_shard: bool = False, hparams: dict[str, Any] | None = None, remote_hf_model_id: str | None = None,
                 disable_mistral_community_chat_template: bool = False,
                 sentence_transformers_dense_modules: bool = False):
        if type(self) is ModelBase or \
                type(self) is TextModel or \
                type(self) is MmprojModel:
            raise TypeError(f"{type(self).__name__!r} should not be directly instantiated")

        if self.is_mistral_format and not _mistral_common_installed:
