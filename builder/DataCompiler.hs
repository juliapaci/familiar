-- first time writing haskell, so i followed along with this until i need to make some project specific changes: https://www.youtube.com/watch?v=N9RUqGYuGfw&t=1345s

module Main where

import Control.Applicative
import Data.Char

data DataNode
    = DataBool Bool
    | DataString String
    | DataBuiltin
    | DataList (String, [DataNode])
    deriving (Show, Eq)

-- TODO: rewrite to give errors some context
newtype Parser a = Parser { runParser :: String -> Maybe (String, a) }

instance Functor Parser where
    fmap f (Parser p) =
        Parser $ \x -> do
            (x', y) <- p x
            Just (x', f y)

instance Applicative Parser where
    pure x = Parser $ \y -> Just (y, x)
    (Parser p1) <*> (Parser p2) =
        Parser $ \x -> do
            (x', f) <- p1 x
            (x'', y) <- p2 x'
            Just (x'', f y)

instance Alternative Parser where
    empty = Parser $ \_ -> Nothing
    (Parser p1) <|> (Parser p2) = Parser $ \x -> p1 x <|> p2 x

charParser :: Char -> Parser Char;
charParser x = Parser f
    where
        f (h:t)
            | h == x     = Just (t, h)
            | otherwise  = Nothing
        f [] = Nothing

stringParser :: String -> Parser String
stringParser = sequenceA . map charParser

spanParser :: (Char -> Bool) -> Parser String
spanParser f =
    Parser $ \x ->
        let (token, rest) = span f x
        in Just (rest, token)

stringLiteral :: Parser String
stringLiteral = spanParser isLetter

sepBy :: Parser a -> Parser b -> Parser [b]
sepBy s e = (:) <$> e <*> many (s *> e) <|> pure []

ws :: Parser String
ws = spanParser isSpace

dataBool :: Parser DataNode
dataBool = f <$> (stringParser "yes" <|> stringParser "no")
    where   f "yes" = DataBool True
            f "no"  = DataBool False
            f _     = undefined

dataString :: Parser DataNode
dataString = DataString <$> stringLiteral

dataList :: Parser DataNode
dataList = (\name _ items -> DataList (name, items)) <$>
    stringLiteral
    <*> (charParser ':' *> ws)
    <*> sepBy (charParser '\n') dataNode


dataNode :: Parser DataNode
dataNode = dataBool <|> dataList <|> dataString

parseFile :: FilePath -> Parser a -> IO (Maybe a)
parseFile path parser = do
    i <- readFile path
    return (snd <$> runParser parser i)

main :: IO ()
main = undefined
