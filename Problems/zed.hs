----------------------------------------
--Brute force solution to assignment 1--
----------------------------------------
----------------------------------------
------------Functionality---------------
--Zed :: ([Int],[Int],[Int],[Int])->IO()
--Zed takes a tuple of four int Lists --
--and outputs the solution to the problem
-----------------------------------------
-----------------------------------------
---------Additional Requirements---------
--0. Maps can be n=>2 but larger then 4 -
--take a very long time------------------
--1. Zed can take incomplete infromation-
--and output one of the possible solutions
--2. Zed naturally outputs solution as a-
--normal sized grid depending on puzzle--
--size------------------------------------
--3. Solution set is made smaller by only-
--considering permutations of n distinct--
--numbers to a row instead of and nxn grid
--with multiple same numbers in a row, old
--slow code is provided in the solution---
--(it is commented out) in the set--------
--generator part of the codebase----------
--4. No interactive functions done--------
--5. parMap is used to compute maps in----
--parallel though it slows down the-------
--computation thus no more parrell--------
--functions where used for efficiencies---
--sake------------------------------------
------------------------------------------

import Data.List
import Data.List.Split
import Control.Parallel
import Control.Parallel.Strategies 
import System.IO
import Data.Char

------------------------
--Solution Set Generator
------------------------

--Slow functions removed from algorithm,
--permutations of large lists has too large
--of a solution set to even do 4x4 grids

--genTableHelp :: Int -> Int -> [Int]
--genTableHelp x 0 = []
--genTableHelp x y = genRow x ++ genTableHelp x (y - 1)

--genTable :: Int -> [Int]
--genTable x = genTableHelp x x

--genPerms :: Int -> [[Int]]
--genPerms x = permutations (genTable x)



--Generates a list from 1 to n 
genRow :: Int -> [Int]
genRow x = [x,(x - 1)..1]

--Finds all permutations of a list from 1 to n
rowPerm :: Int -> [[Int]]
rowPerm x = permutations (genRow x)

--Finds all permutations of nxn lists from 1 to n 
--(which is all possible soluttions) to nxn puzzle
genPerms' :: Int -> [[Int]]
genPerms' x = genPermsHelp 1 x

--Helper function that actually performs above function
genPermsHelp :: Int -> Int -> [[Int]]
genPermsHelp x y
	| y == 1 = zipper 0 row row
	| x == (y - 1) = zipper 0 row row
	| otherwise = zipper 0 row (genPermsHelp (x + 1) y)
		where row = rowPerm y

--Creates a combation of two lists uses zipperHelp but
--this function interates through the elements to create
--all possible combinations
zipper :: Int -> [[Int]] -> [[Int]] -> [[Int]]
zipper x y z
	| x > size = []
	| otherwise = (zipperHelp x y z) ++ (zipper (x + 1) y z) 
		where size = length y

--Creates a combation of two lists x is the element to combine
--With the other list y and z
--Example zipperHelp 0 [[1],[2]] [[1],[2]] = [[1,1],[2,1]]
--Example zipperHelp 1 [[1],[2]] [[1],[2]] = [[1,2],[2,2]]
zipperHelp :: Int -> [[Int]] -> [[Int]] -> [[Int]]
zipperHelp x y z
	| x > ((length y) - 1) = []
	| otherwise = parMap rseq (++(y!!x)) z


------------------------
--Filter of solution set
------------------------

--Checks if solution passes on all rows by feeding a intial function to 
--functions across
validator :: [Int] -> Bool
validator x = across x 0

--Checks if solution passes on all rows with an intial value
across :: [Int] -> Int -> Bool
across x y
	| (size - 1) < y = True
	| otherwise = (numberCheckAcross chunk) && (across x (y + 1))
		where 
			size = (intSquare $ length x)
			chunk = (chunksOf size x)!!y

--Checks if there are any repeats across a possible solution of a single row
numberCheckAcross :: [Int] -> Bool
numberCheckAcross x
	| (length $ (nub x)) /= (length x) = False
	| otherwise = True

-- Returns the solution set filtered by if it is a valid solution to
-- a nxn puzzle 
validFilter :: [[Int]] -> [[Int]] -- main filter
validFilter x = filter validator (parMap rseq transform (filter validator x))

-- Function that returns solution set without repeats of a nxn puzzle
validUniquePerms :: Int -> [[Int]]
validUniquePerms x = nub $ validFilter $ genPerms' x 

intSquare :: Int -> Int -- square root that returns ints
intSquare x = truncate $ sqrt $ fromIntegral y
	where y = x::Int

--Turns a matrix 90 degrees clockwise
transform :: [Int] -> [Int]
transform x = transformHelp x size
	where size = (intSquare $ length x) - 1

transformHelp :: [Int] -> Int -> [Int]
transformHelp x y
	| y < 0 = []
	| otherwise = (transformHelper chunk y 0) ++ (transformHelp x (y - 1))
		where 
			size = intSquare $ length x
			chunk = chunksOf size x

transformHelper :: [[Int]] -> Int -> Int -> [Int]
transformHelper x y z --Arrays, element, position
	| size == z = []
	| otherwise = (x!!z!!y):[] ++ transformHelper x y (z + 1)
		where size = length x

----------------
-- Solve problem 
----------------

--Main function that takes the puzzle tupple and outputs the solution
zed :: ([Int],[Int],[Int],[Int]) -> IO ()
zed x = toGrid $ solve (tuppleToList x) (validUniquePerms size) 0
	where size = length ((tuppleToList x)!!0)

--Checks all solutions and checks them if they are valid
solve :: [[Int]] -> [[Int]] -> Int -> [[Int]]
solve x y z
	| z > (length prep) = error "No solution"
	| (checkWhole x (prep!!z)) == True = (prep!!z)
	| otherwise = solve x y (z + 1)
	where
		prep = parMap rseq (chunksOf size) y 
		size = length (x!!0)

--Checks all the sets of rows of a puzzle and outputs if it a solution
checkWhole :: [[Int]] -> [[Int]] -> Bool
checkWhole x y = first && second && third && fourth
	where
		first = check (reverse (x!!3)) y 0
		second = check (reverse (x!!0)) secondprep 0
		secondprep = chunksOf size $ transform $ toList y
		third = check (reverse (x!!1)) thirdprep 0
		thirdprep = chunksOf size $ transform $ transform $ toList y
		fourth = check (reverse (x!!2)) fourthprep 0
		fourthprep = chunksOf size $ transform $ transform $ transform $ toList y
		size = length (x!!0)

--Checks the set of rows of one side of the puzzle
check :: [Int] -> [[Int]] -> Int -> Bool
check x y z 
	| (z + 1) > length x = True
	| otherwise = (checkAcross (x!!z) (y!!z) 0 0) && (check x y (z + 1))

--Checks a single row of the puzzle
checkAcross :: Int -> [Int] -> Int -> Int -> Bool
checkAcross x y z w
	| x == 0 = True
	| z == 0 =  checkAcross x y (z + 1) (head y)
	| (y == []) && (x == z) = True
	| (y == []) && (x /= z) = False
	| w >= head y = checkAcross x (tail y) z w
	| otherwise = checkAcross x (tail y) (z + 1) (head y)

tuppleToList :: ([Int],[Int],[Int],[Int]) -> [[Int]]
tuppleToList (a,b,c,d) = [a] ++ [b] ++ [c] ++ [d]

--Creates a output grid from a solution list of lists
toGrid :: [[Int]] -> IO ()
toGrid x = mapM_ putStrLn $ (addLines (map toString x) 0)

toList :: [[Int]] -> [Int]
toList [] = []
toList (x:xs) = x ++ toList xs

--Adds padding lines inbetween lists
--Example addLines [["|a|"],["|b|"]] 0 = [["|a|"],["___"],["|b|"]]
addLines :: [[Char]] -> Int -> [[Char]]
addLines x y
	| y > ((length (x)) - 1) = [line]
	| otherwise = [line] ++ [x!!y] ++ (addLines x (y + 1))
		where line = makeLine ((length (x!!0)) - 1) 0

--Creates lines for grid construction x is how long and y is current length
--Example makeLine 2 0 = "__"
makeLine :: Int -> Int -> [Char]
makeLine x y
	| y > x = ""
	| y <= x = "-" ++ (makeLine x (y + 1))
	
toString :: [Int] -> [Char]
toString x = toStringHelp x False

toStringHelp :: [Int] -> Bool -> [Char]
toStringHelp [] _ = []
toStringHelp x False = ['|'] ++ toStringHelp x True
toStringHelp (x:xs) _ = [intToDigit x] ++ ['|'] ++ toStringHelp xs True

