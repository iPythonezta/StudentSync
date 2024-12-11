import logo from './logo.svg';
import './App.css';
import 'bootstrap/dist/css/bootstrap.min.css';
import {Container} from 'react-bootstrap';
import Login from './Components/Login';
import Home from './Components/Home';
import {BrowserRouter as Router, Route, Routes} from 'react-router-dom';
import ContextProvider from './ContexApi';
function App() {
  return (
      <ContextProvider>
        <Router>
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/login" element={<Login />} />
          </Routes>
        </Router>
      </ContextProvider>
  );
}

export default App;
