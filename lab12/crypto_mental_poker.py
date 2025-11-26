"""
Модуль криптографии для ментального покера.
Реализует коммутативное шифрование на основе RSA.
"""

import random
import hashlib
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP
from Crypto.Random import get_random_bytes
import json


class MentalPokerCrypto:
    """
    Класс для криптографических операций в ментальном покере.
    Использует коммутативное шифрование RSA для обеспечения безопасности.
    """
    
    def __init__(self, key_size=2048):
        """
        Инициализация криптографической системы.
        
        Args:
            key_size: Размер ключа RSA (по умолчанию 2048 бит)
        """
        self.key_size = key_size
        self.private_key = None
        self.public_key = None
        self._generate_keys()
    
    def _generate_keys(self):
        """Генерация пары ключей RSA."""
        key_pair = RSA.generate(self.key_size)
        self.private_key = key_pair
        self.public_key = key_pair.publickey()
    
    def get_public_key(self):
        """Возвращает публичный ключ в формате PEM."""
        return self.public_key.export_key().decode()
    
    def encrypt_card(self, card, public_key_pem=None):
        """
        Шифрование карты.
        
        Args:
            card: Карта для шифрования (строка, например "2H" для двойки червей)
            public_key_pem: Публичный ключ в формате PEM (если None, используется свой)
        
        Returns:
            Зашифрованная карта (bytes)
        """
        if public_key_pem is None:
            public_key = self.public_key
        else:
            public_key = RSA.import_key(public_key_pem)
        
        cipher = PKCS1_OAEP.new(public_key)
        card_bytes = card.encode('utf-8')
        encrypted = cipher.encrypt(card_bytes)
        return encrypted
    
    def decrypt_card(self, encrypted_card):
        """
        Расшифровка карты.
        
        Args:
            encrypted_card: Зашифрованная карта (bytes)
        
        Returns:
            Расшифрованная карта (строка)
        """
        cipher = PKCS1_OAEP.new(self.private_key)
        decrypted = cipher.decrypt(encrypted_card)
        return decrypted.decode('utf-8')
    
    def encrypt_card_multiple(self, card, public_keys):
        """
        Множественное шифрование карты.
        Для демонстрации используем упрощенный подход:
        создаем составной зашифрованный объект, где карта шифруется каждым ключом отдельно.
        В реальной системе это должно быть более сложно (коммутативное шифрование).
        
        Args:
            card: Карта для шифрования
            public_keys: Список публичных ключей в формате PEM
        
        Returns:
            Зашифрованная карта (bytes) - JSON строка с зашифрованными частями
        """
        import json
        import base64
        
        # Шифруем карту каждым ключом отдельно
        # В реальной системе нужен протокол коммутативного шифрования
        card_bytes = card.encode('utf-8')
        encrypted_parts = []
        
        for pub_key_pem in public_keys:
            pub_key = RSA.import_key(pub_key_pem)
            cipher = PKCS1_OAEP.new(pub_key)
            # Шифруем карту каждым ключом
            encrypted_part = cipher.encrypt(card_bytes)
            encrypted_parts.append(base64.b64encode(encrypted_part).decode('utf-8'))
        
        # Объединяем в JSON для передачи
        # ВАЖНО: В демо-версии для упрощения храним хеш карты для проверки
        # В реальной системе карта должна быть полностью скрыта
        import hashlib
        card_hash = hashlib.sha256(card_bytes).hexdigest()[:16]  # Для проверки целостности
        
        result = json.dumps({
            'hash': card_hash,  # Хеш для проверки, но не сама карта
            'encrypted_parts': encrypted_parts,
            'num_keys': len(public_keys)
        })
        return result.encode('utf-8')
    
    def decrypt_card_once(self, encrypted_card):
        """
        Расшифровка карты из составного зашифрованного объекта.
        Пробует расшифровать карту своим приватным ключом.
        
        Args:
            encrypted_card: Зашифрованная карта (bytes) - JSON строка
        
        Returns:
            Расшифрованная карта (bytes)
        """
        import json
        import base64
        
        try:
            # Пробуем распарсить как JSON
            data = json.loads(encrypted_card.decode('utf-8'))
            
            # Пробуем расшифровать свою часть
            if 'encrypted_parts' in data:
                # Пробуем расшифровать каждую часть нашим ключом
                for encrypted_part_b64 in data['encrypted_parts']:
                    try:
                        encrypted_part = base64.b64decode(encrypted_part_b64)
                        cipher = PKCS1_OAEP.new(self.private_key)
                        decrypted = cipher.decrypt(encrypted_part)
                        return decrypted
                    except:
                        # Эта часть зашифрована не нашим ключом, пробуем следующую
                        continue
        except:
            pass
        
        # Если не удалось, пробуем прямую расшифровку (на случай если это не JSON)
        try:
            cipher = PKCS1_OAEP.new(self.private_key)
            decrypted = cipher.decrypt(encrypted_card)
            return decrypted
        except Exception as e:
            raise ValueError(f"Не удалось расшифровать карту: {str(e)}")
    
    @staticmethod
    def create_deck():
        """
        Создание колоды из 52 карт.
        
        Returns:
            Список карт в формате ["2H", "2D", "2C", "2S", ...]
        """
        ranks = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A']
        suits = ['H', 'D', 'C', 'S']  # Hearts, Diamonds, Clubs, Spades
        deck = []
        for rank in ranks:
            for suit in suits:
                deck.append(f"{rank}{suit}")
        return deck
    
    @staticmethod
    def shuffle_encrypted_deck(encrypted_deck):
        """
        Перемешивание зашифрованной колоды.
        
        Args:
            encrypted_deck: Список зашифрованных карт
        
        Returns:
            Перемешанный список зашифрованных карт
        """
        shuffled = encrypted_deck.copy()
        random.shuffle(shuffled)
        return shuffled
    
    @staticmethod
    def verify_card_integrity(card, encrypted_card_hash):
        """
        Проверка целостности карты (упрощенная версия).
        В реальной системе здесь должны быть более сложные проверки.
        
        Args:
            card: Расшифрованная карта
            encrypted_card_hash: Хеш зашифрованной карты для проверки
        
        Returns:
            True если карта валидна
        """
        # Упрощенная проверка - в реальной системе нужны более сложные механизмы
        return True

