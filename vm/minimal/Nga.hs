-- Minimal Nga virtual machine for RETRO, implemented in Haskell (GHC).
-- Builds to a single module: ghc Nga.hs -o nga-hs
import qualified Data.ByteString as BS
import Data.Array.IO (IOArray, newArray, readArray, writeArray)
import Data.Bits ((.&.), (.|.), shiftL, shiftR, complement, xor)
import Data.IORef
import Data.Int (Int32)
import Data.Word (Word32)
import Control.Monad (forM_, when)
import System.IO (hFlush, stdout, hIsEOF, stdin)
import System.Exit (exitSuccess)
import Data.Char (chr)

type Cell = Int32

imageSize, addresses, stackDepth :: Int
imageSize = 65536
addresses = 256
stackDepth = 256

cellMin, cellMax :: Cell
cellMin = minBound + 1
cellMax = maxBound - 1

data NgaVm = NgaVm
  { spRef     :: IORef Int
  , rpRef     :: IORef Int
  , ipRef     :: IORef Int
  , dataStack :: IOArray Int Cell
  , addrStack :: IOArray Int Cell
  , memory    :: IOArray Int Cell
  }

newVm :: IO NgaVm
newVm = do
  sp <- newIORef 0
  rp <- newIORef 0
  ip <- newIORef 0
  ds <- newArray (0, stackDepth - 1) 0
  as <- newArray (0, addresses - 1) 0
  mem <- newArray (0, imageSize) 0
  pure NgaVm { spRef = sp, rpRef = rp, ipRef = ip, dataStack = ds, addrStack = as, memory = mem }

tos :: NgaVm -> IO Cell
tos vm = do s <- readIORef (spRef vm); readArray (dataStack vm) s

nos :: NgaVm -> IO Cell
nos vm = do s <- readIORef (spRef vm); readArray (dataStack vm) (s - 1)

tors :: NgaVm -> IO Cell
tors vm = do r <- readIORef (rpRef vm); readArray (addrStack vm) r

setTos :: NgaVm -> Cell -> IO ()
setTos vm v = do s <- readIORef (spRef vm); writeArray (dataStack vm) s v

setNos :: NgaVm -> Cell -> IO ()
setNos vm v = do s <- readIORef (spRef vm); writeArray (dataStack vm) (s - 1) v

setTors :: NgaVm -> Cell -> IO ()
setTors vm v = do r <- readIORef (rpRef vm); writeArray (addrStack vm) r v

stackPush :: NgaVm -> Cell -> IO ()
stackPush vm v = do
  modifyIORef' (spRef vm) (+1)
  s <- readIORef (spRef vm)
  writeArray (dataStack vm) s v

stackPop :: NgaVm -> IO Cell
stackPop vm = do
  s <- readIORef (spRef vm)
  v <- readArray (dataStack vm) s
  modifyIORef' (spRef vm) (subtract 1)
  pure v

execute :: NgaVm -> Int -> IO ()
execute vm cell = do
  writeIORef (rpRef vm) 1
  writeIORef (ipRef vm) cell
  loop
 where
  loop = do
    ip <- readIORef (ipRef vm)
    when (ip < imageSize) $ do
      opcode <- readArray (memory vm) ip
      processOpcodeBundle vm opcode
      modifyIORef' (ipRef vm) (+1)
      rp <- readIORef (rpRef vm)
      when (rp == 0) $ writeIORef (ipRef vm) imageSize
      loop

loadImage :: NgaVm -> FilePath -> IO ()
loadImage vm path = do
  bytes <- BS.readFile path
  let cells = min imageSize (BS.length bytes `div` 4)
  forM_ [0 .. cells - 1] $ \i -> do
    let idx = i * 4
        b0 = fromIntegral (BS.index bytes idx)       :: Word32
        b1 = fromIntegral (BS.index bytes (idx + 1)) :: Word32
        b2 = fromIntegral (BS.index bytes (idx + 2)) :: Word32
        b3 = fromIntegral (BS.index bytes (idx + 3)) :: Word32
        w  = b0 .|. (b1 `shiftL` 8) .|. (b2 `shiftL` 16) .|. (b3 `shiftL` 24)
        c  = fromIntegral w :: Cell
    writeArray (memory vm) i c

prepareVm :: NgaVm -> IO ()
prepareVm vm = do
  writeIORef (ipRef vm) 0
  writeIORef (spRef vm) 0
  writeIORef (rpRef vm) 0
  forM_ [0 .. imageSize] $ \i -> writeArray (memory vm) i 0
  forM_ [0 .. stackDepth - 1] $ \i -> writeArray (dataStack vm) i 0
  forM_ [0 .. addresses - 1] $ \i -> writeArray (addrStack vm) i 0

inst_no :: NgaVm -> IO ()
inst_no _ = pure ()

inst_li :: NgaVm -> IO ()
inst_li vm = do
  modifyIORef' (ipRef vm) (+1)
  ip <- readIORef (ipRef vm)
  v <- readArray (memory vm) ip
  stackPush vm v

inst_du :: NgaVm -> IO ()
inst_du vm = tos vm >>= stackPush vm

inst_dr :: NgaVm -> IO ()
inst_dr vm = do
  s <- readIORef (spRef vm)
  writeArray (dataStack vm) s 0
  modifyIORef' (spRef vm) (subtract 1)
  s' <- readIORef (spRef vm)
  when (s' < 0) $ writeIORef (ipRef vm) imageSize

inst_sw :: NgaVm -> IO ()
inst_sw vm = do
  a <- tos vm
  b <- nos vm
  setTos vm b
  setNos vm a

inst_pu :: NgaVm -> IO ()
inst_pu vm = do
  modifyIORef' (rpRef vm) (+1)
  t <- tos vm
  setTors vm t
  inst_dr vm

inst_po :: NgaVm -> IO ()
inst_po vm = do
  t <- tors vm
  stackPush vm t
  modifyIORef' (rpRef vm) (subtract 1)

inst_ju :: NgaVm -> IO ()
inst_ju vm = do
  t <- tos vm
  writeIORef (ipRef vm) (fromIntegral t - 1)
  inst_dr vm

inst_ca :: NgaVm -> IO ()
inst_ca vm = do
  modifyIORef' (rpRef vm) (+1)
  ip <- readIORef (ipRef vm)
  setTors vm (fromIntegral ip)
  t <- tos vm
  writeIORef (ipRef vm) (fromIntegral t - 1)
  inst_dr vm

inst_cc :: NgaVm -> IO ()
inst_cc vm = do
  a <- tos vm
  inst_dr vm
  b <- tos vm
  inst_dr vm
  when (b /= 0) $ do
    modifyIORef' (rpRef vm) (+1)
    ip <- readIORef (ipRef vm)
    setTors vm (fromIntegral ip)
    writeIORef (ipRef vm) (fromIntegral a - 1)

inst_re :: NgaVm -> IO ()
inst_re vm = do
  t <- tors vm
  writeIORef (ipRef vm) (fromIntegral t)
  modifyIORef' (rpRef vm) (subtract 1)

inst_eq :: NgaVm -> IO ()
inst_eq vm = do
  a <- nos vm
  b <- tos vm
  setNos vm (if a == b then -1 else 0)
  inst_dr vm

inst_ne :: NgaVm -> IO ()
inst_ne vm = do
  a <- nos vm
  b <- tos vm
  setNos vm (if a /= b then -1 else 0)
  inst_dr vm

inst_lt :: NgaVm -> IO ()
inst_lt vm = do
  a <- nos vm
  b <- tos vm
  setNos vm (if a < b then -1 else 0)
  inst_dr vm

inst_gt :: NgaVm -> IO ()
inst_gt vm = do
  a <- nos vm
  b <- tos vm
  setNos vm (if a > b then -1 else 0)
  inst_dr vm

inst_fe :: NgaVm -> IO ()
inst_fe vm = do
  t <- tos vm
  case t of
    -1 -> setTos vm . fromIntegral =<< fmap (subtract 1) (readIORef (spRef vm))
    -2 -> setTos vm . fromIntegral =<< readIORef (rpRef vm)
    -3 -> setTos vm (fromIntegral imageSize)
    -4 -> setTos vm cellMin
    -5 -> setTos vm cellMax
    _  -> do v <- readArray (memory vm) (fromIntegral t); setTos vm v

inst_st :: NgaVm -> IO ()
inst_st vm = do
  addr <- tos vm
  if addr >= 0 && addr <= fromIntegral imageSize
     then do
       val <- nos vm
       writeArray (memory vm) (fromIntegral addr) val
       inst_dr vm
       inst_dr vm
     else writeIORef (ipRef vm) imageSize

inst_ad :: NgaVm -> IO ()
inst_ad vm = do a <- nos vm; b <- tos vm; setNos vm (a + b); inst_dr vm

inst_su :: NgaVm -> IO ()
inst_su vm = do a <- nos vm; b <- tos vm; setNos vm (a - b); inst_dr vm

inst_mu :: NgaVm -> IO ()
inst_mu vm = do a <- nos vm; b <- tos vm; setNos vm (a * b); inst_dr vm

inst_di :: NgaVm -> IO ()
inst_di vm = do
  a <- tos vm
  b <- nos vm
  setTos vm (b `div` a)
  setNos vm (b `mod` a)

inst_an :: NgaVm -> IO ()
inst_an vm = do a <- tos vm; b <- nos vm; setNos vm (a .&. b); inst_dr vm

inst_or :: NgaVm -> IO ()
inst_or vm = do a <- tos vm; b <- nos vm; setNos vm (a .|. b); inst_dr vm

inst_xo :: NgaVm -> IO ()
inst_xo vm = do a <- tos vm; b <- nos vm; setNos vm (a `xor` b); inst_dr vm

inst_sh :: NgaVm -> IO ()
inst_sh vm = do
  y <- tos vm
  x <- nos vm
  let result
        | y < 0     = x `shiftL` fromIntegral (0 - y)
        | x < 0 && y > 0 = (x `shiftR` fromIntegral y) .|. (complement 0 `shiftR` fromIntegral y)
        | otherwise = x `shiftR` fromIntegral y
  setNos vm result
  inst_dr vm

inst_zr :: NgaVm -> IO ()
inst_zr vm = do
  t <- tos vm
  when (t == 0) $ do
    inst_dr vm
    r <- tors vm
    writeIORef (ipRef vm) (fromIntegral r)
    modifyIORef' (rpRef vm) (subtract 1)

inst_ha :: NgaVm -> IO ()
inst_ha vm = writeIORef (ipRef vm) imageSize

inst_ie :: NgaVm -> IO ()
inst_ie vm = stackPush vm 2

inst_iq :: NgaVm -> IO ()
inst_iq vm = do
  t <- tos vm
  case t of
    0 -> do inst_dr vm; stackPush vm 0; stackPush vm 0
    1 -> do inst_dr vm; stackPush vm 1; stackPush vm 1
    _ -> pure ()

inst_ii :: NgaVm -> IO ()
inst_ii vm = do
  t <- tos vm
  case t of
    0 -> do
      inst_dr vm
      c <- stackPop vm
      putChar (chr (fromIntegral (c .&. 0xFF)))
      hFlush stdout
    1 -> do
      eof <- hIsEOF stdin
      when eof exitSuccess
      b <- getChar
      inst_dr vm
      stackPush vm (fromIntegral (fromEnum b .&. 0xFF))
    _ -> inst_dr vm

processOpcodeBundle :: NgaVm -> Cell -> IO ()
processOpcodeBundle vm opcode = do
  executeInstruction vm (fromIntegral (opcode .&. 0xFF))
  executeInstruction vm (fromIntegral ((opcode `shiftR` 8) .&. 0xFF))
  executeInstruction vm (fromIntegral ((opcode `shiftR` 16) .&. 0xFF))
  executeInstruction vm (fromIntegral ((opcode `shiftR` 24) .&. 0xFF))

executeInstruction :: NgaVm -> Int -> IO ()
executeInstruction vm inst = case inst of
  0  -> inst_no vm
  1  -> inst_li vm
  2  -> inst_du vm
  3  -> inst_dr vm
  4  -> inst_sw vm
  5  -> inst_pu vm
  6  -> inst_po vm
  7  -> inst_ju vm
  8  -> inst_ca vm
  9  -> inst_cc vm
  10 -> inst_re vm
  11 -> inst_eq vm
  12 -> inst_ne vm
  13 -> inst_lt vm
  14 -> inst_gt vm
  15 -> inst_fe vm
  16 -> inst_st vm
  17 -> inst_ad vm
  18 -> inst_su vm
  19 -> inst_mu vm
  20 -> inst_di vm
  21 -> inst_an vm
  22 -> inst_or vm
  23 -> inst_xo vm
  24 -> inst_sh vm
  25 -> inst_zr vm
  26 -> inst_ha vm
  27 -> inst_ie vm
  28 -> inst_iq vm
  29 -> inst_ii vm
  _  -> inst_no vm

main :: IO ()
main = do
  vm <- newVm
  prepareVm vm
  loadImage vm "ngaImage"
  execute vm 0
